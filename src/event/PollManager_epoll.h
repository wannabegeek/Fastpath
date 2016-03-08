//
//  TFEventPollManager_epoll.h
//  TFFIXEngine
//
//  Created by Tom Fewster on 12/07/2013.
//  Copyright (c) 2013 Tom Fewster. All rights reserved.
//

#ifndef TFFIXEngine_TFEventPollManager_epoll_h
#define TFFIXEngine_TFEventPollManager_epoll_h

#include "EventType.h"
#include "utils/logger.h"

#include <thread>
#include <sys/epoll.h>
#include <iostream>
#include <cstring>

namespace DCF {
   template <int = MAX_EVENTS> class EventPoll {
   private:
      int epollfd = -1;
      int m_events = 0;
   public:
      EventPoll() {
         epollfd = epoll_create1(0);
         if (epollfd == -1) {
             ERROR_LOG("Failed to create epoll fd: " << strerror(errno));
             return;
         }
      }

      ~EventPoll() {
      }

      bool add(const EventPollElement &event) {
         struct epoll_event ev;
         memset(&ev, 0, sizeof(struct epoll_event));

         int filter = 0;
         if ((event.filter & EventType::READ) == EventType::READ) {
            filter |= EPOLLIN;
         }
         if ((event.filter & EventType::WRITE) == EventType::WRITE) {
            filter |= EPOLLOUT;
         }
         ++m_events;

         ev.events = filter;
         ev.data.fd = event.fd;

         if (epoll_ctl(epollfd, EPOLL_CTL_ADD, event.fd, &ev) == -1) {
             ERROR_LOG("Failed to add event with epoll_ctl: " << strerror(errno));
         }

         return true;
      }

      bool remove(const EventPollElement &event) {
         struct epoll_event ev;
         memset(&ev, 0, sizeof(struct epoll_event));

         int filter = 0;
         if ((event.filter & EventType::READ) == EventType::READ) {
            filter |= EPOLLIN;
         }
         if ((event.filter & EventType::WRITE) == EventType::WRITE) {
            filter |= EPOLLOUT;
         }
         --m_events;

         ev.events = filter;
         ev.data.fd = event.fd;

         if (epoll_ctl(epollfd, EPOLL_CTL_DEL, event.fd, &ev) == -1) {
             if (errno != EBADF) { // probably the fd is already closed (& epoll with automatically remove it)
             	ERROR_LOG("Failed to remove event with epoll_ctl: " << strerror(errno));
                return false;
             }
         }

         return true;
      }

      int run(std::array<EventPollElement, MAX_EVENTS> &events, int &numEvents, const std::chrono::steady_clock::duration &duration) {
         int result = -1;

         if (m_events != 0) {
            long long timeout = -1;
            if (duration != DistantFuture) {
                timeout = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            }

            struct epoll_event _events[MAX_EVENTS];
            result = epoll_wait(epollfd, _events, m_events, timeout);
            if (result == -1) {
                ERROR_LOG("epoll_wait returned -1: " << strerror(errno));
               return -1;
            } else {
               numEvents = 0;
               for (int j = 0; j < result; ++j) {
                  int filter = EventType::NONE;
                  if ((_events[j].events & EPOLLIN) == EPOLLIN) {
                     filter |= EventType::READ;
                  }
                  if ((_events[j].events & EPOLLOUT) == EPOLLOUT) {
                     filter |= EventType::WRITE;
                  }

                  if (filter != EventType::NONE) {
                     events[numEvents++] = EventPollElement(_events[j].data.fd, filter);
                  }
               }
            }
         } else {
            std::this_thread::sleep_for(duration);
         }
         return 0;
      }

      friend std::ostream &operator<<(std::ostream &out, const EventPoll &event) {
         out << "Event loop registrations:\n";
         return out;
      }
   };
}

#endif
