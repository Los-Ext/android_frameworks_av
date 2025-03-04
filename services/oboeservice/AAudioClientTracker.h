/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_AAUDIO_AAUDIO_CLIENT_TRACKER_H
#define ANDROID_AAUDIO_AAUDIO_CLIENT_TRACKER_H

#include <map>
#include <mutex>
#include <set>

#include <android-base/thread_annotations.h>
#include <utils/Singleton.h>

#include <aaudio/AAudio.h>
#include <aaudio/IAAudioClient.h>
#include "AAudioService.h"

namespace aaudio {

class AAudioClientTracker : public android::Singleton<AAudioClientTracker>{
public:
    AAudioClientTracker();
    ~AAudioClientTracker() = default;

    /**
     * Returns information about the state of the this class.
     *
     * Will attempt to get the object lock, but will proceed
     * even if it cannot.
     *
     * Each line of information ends with a newline.
     *
     * @return a string with useful information
     */
    std::string dump() const;

    aaudio_result_t registerClient(pid_t pid, const android::sp<IAAudioClient>& client);

    void unregisterClient(pid_t pid);

    int32_t getStreamCount(pid_t pid);

    aaudio_result_t registerClientStream(pid_t pid,
                                         const android::sp<AAudioServiceStreamBase>& serviceStream);

    aaudio_result_t unregisterClientStream(
            pid_t pid, const android::sp<AAudioServiceStreamBase>& serviceStream);

    /**
     * Specify whether a process is allowed to create an EXCLUSIVE MMAP stream.
     * @param pid
     * @param enabled
     */
    void setExclusiveEnabled(pid_t pid, bool enabled);

    bool isExclusiveEnabled(pid_t pid);

    android::AAudioService *getAAudioService() const {
        return mAAudioService;
    }

    void setAAudioService(android::AAudioService *aaudioService) {
        mAAudioService = aaudioService;
    }

private:

    /**
     * One per process.
     */
    class NotificationClient : public IBinder::DeathRecipient {
    public:
        NotificationClient(pid_t pid, const android::sp<IBinder>& binder);
        ~NotificationClient() override = default;

        int32_t getStreamCount();

        std::string dump() const;

        aaudio_result_t registerClientStream(
                const android::sp<AAudioServiceStreamBase>& serviceStream);

        aaudio_result_t unregisterClientStream(
                const android::sp<AAudioServiceStreamBase>& serviceStream);

        void setExclusiveEnabled(bool enabled) {
            mExclusiveEnabled = enabled;
        }

        bool isExclusiveEnabled() {
            return mExclusiveEnabled;
        }

        bool isBinderNull() {
            return mBinder == nullptr;
        }

        void setBinder(android::sp<IBinder>& binder) {
            mBinder = binder;
        }

        // IBinder::DeathRecipient
        void binderDied(const android::wp<IBinder>& who) override;

    private:
        mutable std::mutex                              mLock;
        const pid_t                                     mProcessId;
        std::set<android::sp<AAudioServiceStreamBase>>  mStreams GUARDED_BY(mLock);
        // hold onto binder to receive death notifications
        android::sp<IBinder>                            mBinder;
        bool                                            mExclusiveEnabled = true;
    };

    // This must be called under mLock
    android::sp<NotificationClient> getNotificationClient_l(pid_t pid)
            REQUIRES(mLock);

    mutable std::mutex                               mLock;
    std::map<pid_t, android::sp<NotificationClient>> mNotificationClients
            GUARDED_BY(mLock);
    android::AAudioService                          *mAAudioService = nullptr;
};

} /* namespace aaudio */

#endif //ANDROID_AAUDIO_AAUDIO_CLIENT_TRACKER_H
