//
// Created by fewstert on 01/04/16.
//

#include <iostream>
#include <chrono>
#include <thread>
#include "fastpath/utils/logger.h"
#include "fastpath/event/InlineQueue.h"
#include "fastpath/event/BlockingQueue.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <boost/interprocess/shared_memory_object.hpp>
#pragma GCC diagnostic pop
#include "fastpath/transport/sm/InterprocessNotifierServer.h"
#include "fastpath/SharedMemoryBuffer.h"
#include "fastpath/messages/MutableMessage.h"
#include "fastpath/messages/Message.h"
#include "fastpath/messages/MessageCodec.h"
#include "fastpath/utils/tfpool.h"
#include "fastpath/utils/tfnulllock.h"
#include "fastpath/utils/temp_directory.h"
#include "fastpath/event/SignalEvent.h"

struct PeerConnection {
    fp::InterprocessNotifierServer::notifier_type m_notifier;

    std::unique_ptr<fp::SharedMemoryBuffer> m_clientQueue = nullptr;
    std::unique_ptr<fp::UnixSocket> m_socket = nullptr;
    int m_process_id;

    fp::DataEvent *m_socketEvent = nullptr;
    fp::DataEvent *m_notifierEvent = nullptr;

    std::function<void(PeerConnection *connection)> m_callback;
    std::function<void(PeerConnection *connection)> m_msg_callback;

    PeerConnection(fp::InterprocessNotifierServer::notifier_type &&notifier, std::unique_ptr<fp::UnixSocket> &&socket, int process_id, fp::SharedMemoryManager *sm_manager)
            : m_notifier(std::forward<fp::InterprocessNotifierServer::notifier_type>(notifier)),
              m_socket(std::forward<std::unique_ptr<fp::UnixSocket>>(socket)),
              m_process_id(process_id) {

        char queueName[32];
        ::sprintf(queueName, "ClientQueue_%i", process_id);
        m_clientQueue = std::make_unique<fp::SharedMemoryBuffer>(queueName, *sm_manager);
    }

    PeerConnection(PeerConnection &&other) noexcept
            : m_notifier(std::move(other.m_notifier)),
              m_clientQueue(std::move(other.m_clientQueue)),
              m_socket(std::move(other.m_socket)),
              m_process_id(std::move(other.m_process_id)),
              m_socketEvent(std::move(other.m_socketEvent)),
              m_notifierEvent(std::move(other.m_notifierEvent)),
              m_callback(std::move(other.m_callback)) {
    }

    ~PeerConnection() noexcept {
        INFO_LOG("Removing PeerConnection: " << m_process_id);
    }

    PeerConnection &operator=(PeerConnection &&other) noexcept {
        m_notifier = std::move(other.m_notifier);
        m_clientQueue = std::move(other.m_clientQueue);
        m_socket = std::move(other.m_socket);
        m_process_id = std::move(other.m_process_id);
        m_socketEvent = std::move(other.m_socketEvent);
        m_notifierEvent = std::move(other.m_notifierEvent);
        m_callback = std::move(other.m_callback);

        return *this;
    }

    void setPeerDisconnectedHandler(const std::function<void(PeerConnection *connection)> &callback) {
        m_callback = callback;
    }

    void setPeerMessageHandler(const std::function<void(PeerConnection *connection)> &callback) {
        m_msg_callback = callback;
    }

    void notificationHandler(fp::DataEvent *event, const fp::EventType type) noexcept {
        auto &notifier = std::get<0>(m_notifier);
        if (!notifier->reset()) {
            ERROR_LOG("BOOM - Client disconnected: " << m_process_id);
            if (m_callback) {
                m_callback(this);
            }
        } else {
            INFO_LOG("We have been notified of a message: " << m_process_id);
            m_msg_callback(this);
        }
    }

    void socketEvent(fp::Queue *queue) {
        if (!m_socketEvent) {
            m_socketEvent = queue->registerEvent(m_socket->getSocket(), fp::EventType::READ, [&](fp::DataEvent *event, const fp::EventType type) {
                const char buffer[256] = {0};
                ssize_t len = 0;
                while (true) {
                    fp::Socket::ReadResult r = m_socket->read(buffer, 255, len);
                    switch (r) {
                        case fp::Socket::MoreData:
                            break;
                        case fp::Socket::NoData:
                            break;
                        case fp::Socket::Closed: {
                            ERROR_LOG("BOOM - Client disconnected: " << m_process_id);
                            if (m_callback) {
                                m_callback(this);
                            }

                            return;
                        }
                    }
                }
            });
        }
    }

    void notificationEvent(fp::Queue *queue) {
        if (!m_notifierEvent) {
            int fd = std::get<0>(m_notifier)->read_handle();
            DEBUG_LOG("Adding callback for " << fd);
            m_notifierEvent = queue->registerEvent(fd, fp::EventType::READ, std::bind(&PeerConnection::notificationHandler, this, std::placeholders::_1, std::placeholders::_2));
        }
    }
};


class ServerTransport {
private:
    typedef tf::pool<fp::Message, tf::nulllock> PoolType;

    fp::SharedMemoryManager sm_manager;
    fp::SharedMemoryBuffer serverQueue;
    std::vector<std::unique_ptr<PeerConnection>> m_peer_connections;
    std::unique_ptr<fp::InterprocessNotifierServer> notification_server;

    PoolType pool;

    fp::Queue *m_dispatchQueue;

    fp::DataEvent *globalConnectionHandler = nullptr;
    bool m_shutdown = false;
public:

    ServerTransport(fp::Queue *queue) : sm_manager("fprouter_6969", true), serverQueue("ServerQueue", sm_manager), pool(3), m_dispatchQueue(queue) {
        std::string ipc_file = tf::get_temp_directory();
        ipc_file.append("fprouter_");
        ipc_file.append("6969");

        notification_server = std::make_unique<fp::InterprocessNotifierServer>(ipc_file.c_str(), std::bind(&ServerTransport::connectionHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        globalConnectionHandler = notification_server->createReceiverEvent(m_dispatchQueue);
    }

    ~ServerTransport() {
        m_dispatchQueue->unregisterEvent(globalConnectionHandler);
    }

    bool shutdown() const noexcept {
        return m_shutdown;
    }

    void disconnectPeer(PeerConnection *peer) noexcept {
        auto it = std::find_if(m_peer_connections.begin(), m_peer_connections.end(), [&](auto &p) {
            return p.get() == peer;
        });
        if (it != m_peer_connections.end()) {
            INFO_LOG("Client " << peer->m_process_id << " disconnected ");
            if (!m_dispatchQueue->unregisterEvent(peer->m_notifierEvent)) {
                ERROR_LOG("Failed to remove notifierEvent");
            }
            if (m_dispatchQueue->unregisterEvent(peer->m_socketEvent)) {
                ERROR_LOG("Failed to remove socketEvent");
            }
            m_peer_connections.erase(it);
        } else {
            DEBUG_LOG("Failed to find client to remove");
        }

        INFO_LOG("After remove we now have " << m_peer_connections.size() << " connected clients");
//        if (m_peer_connections.size() == 0) {
//            DEBUG_LOG("Setting shutdown flag no clients left");
//            m_shutdown = true;
//        }
    }

    void messageHandler(PeerConnection *connection) noexcept {
        serverQueue.retrieve([&](auto &ptr, fp::SharedMemoryManager::shared_ptr_type &shared_ptr) {

            const char *subject_ptr = nullptr;
            size_t subject_length = 0;
            uint8_t flags = 0;
            size_t msg_length = 0;

            ptr.mark();
            auto status = fp::MessageCodec::addressing_details(ptr, &subject_ptr, subject_length, flags, msg_length);
            if (status == fp::MessageCodec::CompleteMessage) {
                INFO_LOG("Subject is: " << std::string(subject_ptr, subject_length));
            }
            ptr.resetRead();

            fp::Message *msg = pool.allocate();
            fp::MessageCodec::decode(msg, ptr);
            INFO_LOG("Received data '" << *msg);

            fp::MutableMessage msg2;
            msg2.setSubject("SOME.TEST.REPLY");
            uint32_t v = 0;
            msg->getScalarField("TEST", v);
            msg2.addScalarField("RESPONSE", v);
            msg2.addDataField("Name", "Tom");
            msg2.addDataField("Name2", "Zac");

            fp::SharedMemoryBuffer::mutable_storage_type storage(2048, sm_manager.allocator());
            fp::MessageCodec::encode(&msg2, storage);

            fp::SharedMemoryManager::shared_ptr_type sending_buffer = sm_manager.getInterprocessBuffer(&storage);
            std::for_each(m_peer_connections.begin(), m_peer_connections.end(), [&sending_buffer, &shared_ptr](const auto &p) {
                INFO_LOG("Sending to: " << p->m_process_id);
                p->m_clientQueue->notify(shared_ptr);
                p->m_clientQueue->notify(sending_buffer);
                std::get<1>(p->m_notifier)->notify();
            });

            pool.release(msg);
        });
    }

    void connectionHandler(fp::InterprocessNotifierServer::notifier_type &&notifier, std::unique_ptr<fp::UnixSocket> &&socket, int process_id) {
        m_peer_connections.emplace_back(std::make_unique<PeerConnection>(std::move(notifier), std::move(socket), process_id, &sm_manager));

        INFO_LOG("We now have " << m_peer_connections.size() << " connected clients");

        auto &peer = m_peer_connections.back();

        peer->setPeerDisconnectedHandler(std::bind(&ServerTransport::disconnectPeer, this, std::placeholders::_1));
        peer->setPeerMessageHandler(std::bind(&ServerTransport::messageHandler, this, std::placeholders::_1));

        peer->socketEvent(m_dispatchQueue);
        peer->notificationEvent(m_dispatchQueue);
    }

};

int main(int argc, char *argv[]) {

    LOG_LEVEL(tf::logger::debug);
    fp::Session::initialise();

    fp::InlineEventManager evm;
    fp::InlineQueue queue;

    try {
        bool shutdown = false;

        ServerTransport transport(&queue);

        auto fn = [&] (fp::SignalEvent *event, int signal) {
            ERROR_LOG("Shutdown signal caught");
            shutdown = true;
        };

        queue.registerEvent(SIGINT, fn);
        queue.registerEvent(SIGTERM, fn);

        while(!transport.shutdown() && !shutdown) {
            queue.dispatch();
        };
        DEBUG_LOG("Event loop dropped out");

    } catch (const fp::socket_error &e) {
        ERROR_LOG("BOOM - it's broken: " << e.what());
    }

    fp::Session::destroy();
}
