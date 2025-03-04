/*
**
** Copyright (C) 2008 The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <inttypes.h>
#include <stdint.h>
#include <sys/types.h>

#include <android/IDataSource.h>
#include <binder/Parcel.h>
#include <media/IMediaHTTPService.h>
#include <media/IMediaMetadataRetriever.h>
#include <utils/String8.h>
#include <utils/KeyedVector.h>

namespace android {

enum {
    DISCONNECT = IBinder::FIRST_CALL_TRANSACTION,
    SET_DATA_SOURCE_URL,
    SET_DATA_SOURCE_FD,
    SET_DATA_SOURCE_CALLBACK,
    GET_FRAME_AT_TIME,
    GET_IMAGE_AT_INDEX,
    GET_IMAGE_RECT_AT_INDEX,
    GET_FRAME_AT_INDEX,
    EXTRACT_ALBUM_ART,
    EXTRACT_METADATA,
};

class BpMediaMetadataRetriever: public BpInterface<IMediaMetadataRetriever>
{
public:
    explicit BpMediaMetadataRetriever(const sp<IBinder>& impl)
        : BpInterface<IMediaMetadataRetriever>(impl)
    {
    }

    // disconnect from media metadata retriever service
    void disconnect()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaMetadataRetriever::getInterfaceDescriptor());
        remote()->transact(DISCONNECT, data, &reply);
    }

    status_t setDataSource(
            const sp<IMediaHTTPService> &httpService,
            const char *srcUrl,
            const KeyedVector<String8, String8> *headers)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaMetadataRetriever::getInterfaceDescriptor());
        data.writeInt32(httpService != NULL);
        if (httpService != NULL) {
            data.writeStrongBinder(IInterface::asBinder(httpService));
        }
        data.writeCString(srcUrl);

        if (headers == NULL) {
            data.writeInt32(0);
        } else {
            // serialize the headers
            data.writeInt32(headers->size());
            for (size_t i = 0; i < headers->size(); ++i) {
                data.writeString8(headers->keyAt(i));
                data.writeString8(headers->valueAt(i));
            }
        }

        remote()->transact(SET_DATA_SOURCE_URL, data, &reply);
        return reply.readInt32();
    }

    status_t setDataSource(int fd, int64_t offset, int64_t length)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaMetadataRetriever::getInterfaceDescriptor());
        data.writeFileDescriptor(fd);
        data.writeInt64(offset);
        data.writeInt64(length);
        remote()->transact(SET_DATA_SOURCE_FD, data, &reply);
        return reply.readInt32();
    }

    status_t setDataSource(const sp<IDataSource>& source, const char *mime)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaMetadataRetriever::getInterfaceDescriptor());
        data.writeStrongBinder(IInterface::asBinder(source));

        if (mime != NULL) {
            data.writeInt32(1);
            data.writeCString(mime);
        } else {
            data.writeInt32(0);
        }
        remote()->transact(SET_DATA_SOURCE_CALLBACK, data, &reply);
        return reply.readInt32();
    }

    sp<IMemory> getFrameAtTime(int64_t timeUs, int option, int colorFormat, bool metaOnly)
    {
        ALOGV("getTimeAtTime: time(%" PRId64 " us), option(%d), colorFormat(%d) metaOnly(%d)",
                timeUs, option, colorFormat, metaOnly);
        Parcel data, reply;
        data.writeInterfaceToken(IMediaMetadataRetriever::getInterfaceDescriptor());
        data.writeInt64(timeUs);
        data.writeInt32(option);
        data.writeInt32(colorFormat);
        data.writeInt32(metaOnly);
        remote()->transact(GET_FRAME_AT_TIME, data, &reply);
        status_t ret = reply.readInt32();
        if (ret != NO_ERROR) {
            return NULL;
        }
        return interface_cast<IMemory>(reply.readStrongBinder());
    }

    sp<IMemory> getImageAtIndex(int index, int colorFormat, bool metaOnly, bool thumbnail)
    {
        ALOGV("getImageAtIndex: index %d, colorFormat(%d) metaOnly(%d) thumbnail(%d)",
                index, colorFormat, metaOnly, thumbnail);
        Parcel data, reply;
        data.writeInterfaceToken(IMediaMetadataRetriever::getInterfaceDescriptor());
        data.writeInt32(index);
        data.writeInt32(colorFormat);
        data.writeInt32(metaOnly);
        data.writeInt32(thumbnail);
        remote()->transact(GET_IMAGE_AT_INDEX, data, &reply);
        status_t ret = reply.readInt32();
        if (ret != NO_ERROR) {
            return NULL;
        }
        return interface_cast<IMemory>(reply.readStrongBinder());
    }

    sp<IMemory> getImageRectAtIndex(
            int index, int colorFormat, int left, int top, int right, int bottom)
    {
        ALOGV("getImageRectAtIndex: index %d, colorFormat(%d) rect {%d, %d, %d, %d}",
                index, colorFormat, left, top, right, bottom);
        Parcel data, reply;
        data.writeInterfaceToken(IMediaMetadataRetriever::getInterfaceDescriptor());
        data.writeInt32(index);
        data.writeInt32(colorFormat);
        data.writeInt32(left);
        data.writeInt32(top);
        data.writeInt32(right);
        data.writeInt32(bottom);
        remote()->transact(GET_IMAGE_RECT_AT_INDEX, data, &reply);
        status_t ret = reply.readInt32();
        if (ret != NO_ERROR) {
            return NULL;
        }
        return interface_cast<IMemory>(reply.readStrongBinder());
    }

    sp<IMemory> getFrameAtIndex(
            int index, int colorFormat, bool metaOnly)
    {
        ALOGV("getFrameAtIndex: index(%d), colorFormat(%d) metaOnly(%d)",
                index, colorFormat, metaOnly);
        Parcel data, reply;
        data.writeInterfaceToken(IMediaMetadataRetriever::getInterfaceDescriptor());
        data.writeInt32(index);
        data.writeInt32(colorFormat);
        data.writeInt32(metaOnly);
        remote()->transact(GET_FRAME_AT_INDEX, data, &reply);
        status_t ret = reply.readInt32();
        if (ret != NO_ERROR) {
            return NULL;
        }
        return interface_cast<IMemory>(reply.readStrongBinder());
    }

    sp<IMemory> extractAlbumArt()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaMetadataRetriever::getInterfaceDescriptor());
        remote()->transact(EXTRACT_ALBUM_ART, data, &reply);
        status_t ret = reply.readInt32();
        if (ret != NO_ERROR) {
            return NULL;
        }
        return interface_cast<IMemory>(reply.readStrongBinder());
    }

    const char* extractMetadata(int keyCode)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMediaMetadataRetriever::getInterfaceDescriptor());
        data.writeInt32(keyCode);
        remote()->transact(EXTRACT_METADATA, data, &reply);
        status_t ret = reply.readInt32();
        if (ret != NO_ERROR) {
            return NULL;
        }
        const char* str = reply.readCString();
        if (str != NULL) {
            String8 value(str);
            if (mMetadata.indexOfKey(keyCode) < 0) {
                mMetadata.add(keyCode, value);
            } else {
                mMetadata.replaceValueFor(keyCode, value);
            }
            return mMetadata.valueFor(keyCode).c_str();
        } else {
            return NULL;
        }
    }

private:
    KeyedVector<int, String8> mMetadata;
};

IMPLEMENT_META_INTERFACE(MediaMetadataRetriever, "android.media.IMediaMetadataRetriever");

// ----------------------------------------------------------------------

status_t BnMediaMetadataRetriever::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code) {
        case DISCONNECT: {
            CHECK_INTERFACE(IMediaMetadataRetriever, data, reply);
            disconnect();
            return NO_ERROR;
        } break;
        case SET_DATA_SOURCE_URL: {
            CHECK_INTERFACE(IMediaMetadataRetriever, data, reply);

            sp<IMediaHTTPService> httpService;
            if (data.readInt32()) {
                httpService =
                    interface_cast<IMediaHTTPService>(data.readStrongBinder());
            }

            const char* srcUrl = data.readCString();

            if (httpService == NULL || srcUrl == NULL) {
                reply->writeInt32(BAD_VALUE);
                return NO_ERROR;
            }

            KeyedVector<String8, String8> headers;
            size_t numHeaders = (size_t) data.readInt32();
            for (size_t i = 0; i < numHeaders; ++i) {
                String8 key;
                String8 value;
                status_t status;
                status = data.readString8(&key);
                if (status != OK) {
                    return status;
                }
                status = data.readString8(&value);
                if (status != OK) {
                    return status;
                }
                if (headers.add(key, value) < 0) {
                    return UNKNOWN_ERROR;
                }
            }

            reply->writeInt32(
                    setDataSource(
                        httpService, srcUrl, numHeaders > 0 ? &headers : NULL));

            return NO_ERROR;
        } break;
        case SET_DATA_SOURCE_FD: {
            CHECK_INTERFACE(IMediaMetadataRetriever, data, reply);
            int fd = data.readFileDescriptor();
            int64_t offset = data.readInt64();
            int64_t length = data.readInt64();
            reply->writeInt32(setDataSource(fd, offset, length));
            return NO_ERROR;
        } break;
        case SET_DATA_SOURCE_CALLBACK: {
            CHECK_INTERFACE(IMediaMetadataRetriever, data, reply);
            sp<IDataSource> source =
                interface_cast<IDataSource>(data.readStrongBinder());
            if (source == NULL) {
                reply->writeInt32(BAD_VALUE);
            } else {
                int32_t hasMime = data.readInt32();
                const char *mime = NULL;
                if (hasMime) {
                    mime = data.readCString();
                }
                reply->writeInt32(setDataSource(source, mime));
            }
            return NO_ERROR;
        } break;
        case GET_FRAME_AT_TIME: {
            CHECK_INTERFACE(IMediaMetadataRetriever, data, reply);
            int64_t timeUs = data.readInt64();
            int option = data.readInt32();
            int colorFormat = data.readInt32();
            bool metaOnly = (data.readInt32() != 0);
            ALOGV("getTimeAtTime: time(%" PRId64 " us), option(%d), colorFormat(%d), metaOnly(%d)",
                    timeUs, option, colorFormat, metaOnly);
            sp<IMemory> bitmap = getFrameAtTime(timeUs, option, colorFormat, metaOnly);
            if (bitmap != 0) {  // Don't send NULL across the binder interface
                reply->writeInt32(NO_ERROR);
                reply->writeStrongBinder(IInterface::asBinder(bitmap));
            } else {
                reply->writeInt32(UNKNOWN_ERROR);
            }
            return NO_ERROR;
        } break;
        case GET_IMAGE_AT_INDEX: {
            CHECK_INTERFACE(IMediaMetadataRetriever, data, reply);
            int index = data.readInt32();
            int colorFormat = data.readInt32();
            bool metaOnly = (data.readInt32() != 0);
            bool thumbnail = (data.readInt32() != 0);
            ALOGV("getImageAtIndex: index(%d), colorFormat(%d), metaOnly(%d), thumbnail(%d)",
                    index, colorFormat, metaOnly, thumbnail);
            sp<IMemory> bitmap = getImageAtIndex(index, colorFormat, metaOnly, thumbnail);
            if (bitmap != 0) {  // Don't send NULL across the binder interface
                reply->writeInt32(NO_ERROR);
                reply->writeStrongBinder(IInterface::asBinder(bitmap));
            } else {
                reply->writeInt32(UNKNOWN_ERROR);
            }
            return NO_ERROR;
        } break;

        case GET_IMAGE_RECT_AT_INDEX: {
            CHECK_INTERFACE(IMediaMetadataRetriever, data, reply);
            int index = data.readInt32();
            int colorFormat = data.readInt32();
            int left = data.readInt32();
            int top = data.readInt32();
            int right = data.readInt32();
            int bottom = data.readInt32();
            ALOGV("getImageRectAtIndex: index(%d), colorFormat(%d), rect {%d, %d, %d, %d}",
                    index, colorFormat, left, top, right, bottom);
            sp<IMemory> bitmap = getImageRectAtIndex(
                    index, colorFormat, left, top, right, bottom);
            if (bitmap != 0) {  // Don't send NULL across the binder interface
                reply->writeInt32(NO_ERROR);
                reply->writeStrongBinder(IInterface::asBinder(bitmap));
            } else {
                reply->writeInt32(UNKNOWN_ERROR);
            }
            return NO_ERROR;
        } break;

        case GET_FRAME_AT_INDEX: {
            CHECK_INTERFACE(IMediaMetadataRetriever, data, reply);
            int index = data.readInt32();
            int colorFormat = data.readInt32();
            bool metaOnly = (data.readInt32() != 0);
            ALOGV("getFrameAtIndex: index(%d), colorFormat(%d), metaOnly(%d)",
                    index, colorFormat, metaOnly);
            sp<IMemory> frame = getFrameAtIndex(index, colorFormat, metaOnly);
            if (frame != nullptr) {  // Don't send NULL across the binder interface
                reply->writeInt32(NO_ERROR);
                reply->writeStrongBinder(IInterface::asBinder(frame));
            } else {
                reply->writeInt32(UNKNOWN_ERROR);
            }
            return NO_ERROR;
        } break;
        case EXTRACT_ALBUM_ART: {
            CHECK_INTERFACE(IMediaMetadataRetriever, data, reply);
            sp<IMemory> albumArt = extractAlbumArt();
            if (albumArt != 0) {  // Don't send NULL across the binder interface
                reply->writeInt32(NO_ERROR);
                reply->writeStrongBinder(IInterface::asBinder(albumArt));
            } else {
                reply->writeInt32(UNKNOWN_ERROR);
            }
            return NO_ERROR;
        } break;
        case EXTRACT_METADATA: {
            CHECK_INTERFACE(IMediaMetadataRetriever, data, reply);
            int keyCode = data.readInt32();
            const char* value = extractMetadata(keyCode);
            if (value != NULL) {  // Don't send NULL across the binder interface
                reply->writeInt32(NO_ERROR);
                reply->writeCString(value);
            } else {
                reply->writeInt32(UNKNOWN_ERROR);
            }
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

} // namespace android
