//
// Created by Tom Fewster on 23/05/2016.
//

#ifndef FASTPATH_SHM_PEER_CONNECTION_H
#define FASTPATH_SHM_PEER_CONNECTION_H

#include "fastpath/router/peer_connection.h"
#include "fastpath/transport/sm/InterprocessNotifierServer.h"

namespace fp {
    class Queue;
    class SharedMemoryBuffer;
    class UnixSocket;
    class DataEvent;
    class SharedMemoryManager;

    class shm_peer_connection : public peer_connection {
        InterprocessNotifierServer::notifier_type m_notifier;

        std::unique_ptr<SharedMemoryBuffer> m_clientQueue;
        std::unique_ptr<SharedMemoryBuffer> m_serverQueue;

        std::unique_ptr<UnixSocket> m_socket;
        int m_process_id;

        DataEvent *m_socketEvent = nullptr;
        DataEvent *m_notifierEvent = nullptr;

//        std::function<void(PeerConnection *connection)> m_callback;
//        std::function<void(PeerConnection *connection)> m_msg_callback;
        void socketEvent();
        void notificationEvent();

        void notificationHandler(fp::DataEvent *event, const fp::EventType type) noexcept;
    public:

        shm_peer_connection(Queue *queue, InterprocessNotifierServer::notifier_type &&notifier, std::unique_ptr<UnixSocket> &&socket, int process_id, SharedMemoryManager *manager);
        shm_peer_connection(shm_peer_connection &&other) noexcept;
        ~shm_peer_connection() noexcept;

        bool sendBuffer(const message_wrapper &buffer) noexcept override;

        int get_process_id() const noexcept { return m_process_id; }
    };
}

#endif //FASTPATH_SHM_PEER_CONNECTION_H
