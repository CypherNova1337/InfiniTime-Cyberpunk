#include "components/ble/IntrusionLog.h"

#include <algorithm>
#include <cstring>
#include "components/fs/FS.h"

using namespace Pinetime::Controllers;

IntrusionLog::IntrusionLog(FS& fs) : fs {fs} {
}

void IntrusionLog::Load() {
  FileData data;
  lfs_file_t file;
  if (fs.FileOpen(&file, fileName, LFS_O_RDONLY) != LFS_ERR_OK) {
    return;
  }
  const int read = fs.FileRead(&file, reinterpret_cast<uint8_t*>(&data), sizeof(data));
  fs.FileClose(&file);
  if (read != sizeof(data) || data.magic != magic || data.version != fileVersion || data.count > MaxEntries || data.head >= MaxEntries) {
    return;
  }
  enabled = data.enabled != 0;
  count = data.count;
  head = data.head;
  entries = data.entries;
}

void IntrusionLog::Save() {
  FileData data;
  taskENTER_CRITICAL();
  data.magic = magic;
  data.version = fileVersion;
  data.enabled = enabled ? 1 : 0;
  data.count = count;
  data.head = head;
  data.entries = entries;
  dirty = false;
  taskEXIT_CRITICAL();

  lfs_file_t file;
  if (fs.FileOpen(&file, fileName, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC) != LFS_ERR_OK) {
    dirty = true;
    return;
  }
  fs.FileWrite(&file, reinterpret_cast<const uint8_t*>(&data), sizeof(data));
  fs.FileClose(&file);
}

void IntrusionLog::SetEnabled(bool enable) {
  enabled = enable;
  dirty = true;
}

void IntrusionLog::Clear() {
  taskENTER_CRITICAL();
  count = 0;
  committed = false;
  dirty = true;
  taskEXIT_CRITICAL();
}

IntrusionLog::Entry IntrusionLog::Get(size_t index) const {
  Entry entry {};
  taskENTER_CRITICAL();
  if (index < count) {
    entry = entries[(head + MaxEntries - index) % MaxEntries];
  }
  taskEXIT_CRITICAL();
  return entry;
}

void IntrusionLog::FormatAddress(const Entry& entry, char* buffer) {
  static constexpr char hex[] = "0123456789ABCDEF";
  // NimBLE stores addresses little-endian, print them the usual way round
  for (int i = 0; i < 6; i++) {
    const uint8_t byte = entry.address[5 - i];
    *buffer++ = hex[byte >> 4];
    *buffer++ = hex[byte & 0x0F];
    *buffer++ = i < 5 ? ':' : '\0';
  }
}

const char* IntrusionLog::Describe(const Entry& entry) {
  if ((entry.flags & Trusted) != 0) {
    return "BONDED";
  }
  if ((entry.flags & EncryptFailed) != 0) {
    return "PAIR FAILED";
  }
  if ((entry.flags & PairAttempt) != 0) {
    return "PAIR ATTEMPT";
  }
  if ((entry.flags & NoBond) != 0) {
    return "UNVERIFIED";
  }
  return "UNKNOWN";
}

void IntrusionLog::Commit() {
  taskENTER_CRITICAL();
  head = (head + 1) % MaxEntries;
  count = std::min<uint8_t>(count + 1, MaxEntries);
  entries[head] = current;
  committed = true;
  dirty = true;
  taskEXIT_CRITICAL();
}

void IntrusionLog::OnConnect(const uint8_t* address, uint8_t addressType, bool knownBond, uint32_t time) {
  // Nothing to log when disabled, or when the bonded phone reconnects with its identity address
  connected = enabled && !knownBond;
  pending = connected;
  committed = false;
  if (!connected) {
    return;
  }
  current.time = time;
  current.duration = 0;
  std::memcpy(current.address, address, sizeof(current.address));
  current.addressType = addressType;
  current.flags = 0;
  connectTick = xTaskGetTickCount();
}

bool IntrusionLog::OnPairingRequest() {
  if (connected) {
    Active().flags |= PairAttempt;
  }
  return false;
}

bool IntrusionLog::OnEncryptionChange(bool success, bool bonded) {
  if (!connected) {
    return false;
  }
  if (success && bonded) {
    if (pending) {
      // Our own phone: forget about it
      connected = false;
      pending = false;
    } else {
      // Encrypted after it was already reported, keep the record but mark it
      Active().flags |= Trusted;
      dirty = true;
    }
    return false;
  }
  if (!success) {
    Active().flags |= EncryptFailed;
    if (pending) {
      pending = false;
      current.flags |= Alerted;
      Commit();
      return true;
    }
    dirty = true;
  }
  return false;
}

bool IntrusionLog::ClassifyPending(bool hasBond) {
  if (!pending) {
    return false;
  }
  pending = false;
  if (!hasBond) {
    current.flags |= NoBond;
  }
  const bool alert = hasBond || (current.flags & (EncryptFailed | PairAttempt)) != 0;
  if (alert) {
    current.flags |= Alerted;
  }
  Commit();
  return alert;
}

bool IntrusionLog::Classify(bool hasBond) {
  // Give someone typing the passkey on their phone time to finish pairing
  if (pending && (current.flags & PairAttempt) != 0) {
    return false;
  }
  return ClassifyPending(hasBond);
}

bool IntrusionLog::OnDisconnect(bool hasBond) {
  if (!connected) {
    return false;
  }
  const uint32_t seconds = (xTaskGetTickCount() - connectTick) / configTICK_RATE_HZ;
  const uint16_t duration = static_cast<uint16_t>(std::min<uint32_t>(std::max<uint32_t>(seconds, 1), UINT16_MAX));
  current.duration = duration;
  const bool alert = ClassifyPending(hasBond);
  if (committed) {
    taskENTER_CRITICAL();
    entries[head].duration = duration;
    taskEXIT_CRITICAL();
    dirty = true;
  }
  connected = false;
  return alert;
}
