#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <FreeRTOS.h>
#include <task.h>

namespace Pinetime {
  namespace Controllers {
    class FS;

    // Records Bluetooth connections that don't come from the bonded phone.
    //
    // A connection is considered trusted when it encrypts with the stored bond.
    // Anything else is logged, and raises an alert when a bond exists (so there is
    // a known phone to compare against) or when pairing/encryption was attempted.
    class IntrusionLog {
    public:
      static constexpr uint8_t MaxEntries = 16;

      enum Flags : uint8_t {
        Trusted = 1 << 0, // Encrypted with the stored bond
        EncryptFailed = 1 << 1,
        PairAttempt = 1 << 2, // Asked the watch to show a pairing passkey
        Alerted = 1 << 3,
        NoBond = 1 << 4, // No bonded phone to compare against, can't tell friend from foe
      };

      struct Entry {
        uint32_t time;     // Local time, seconds since epoch
        uint16_t duration; // Seconds connected, 0 while still connected
        uint8_t address[6];
        uint8_t addressType;
        uint8_t flags;
      };

      explicit IntrusionLog(FS& fs);

      void Load();
      void Save();

      bool IsEnabled() const {
        return enabled;
      }

      void SetEnabled(bool enable);
      void Clear();

      bool IsDirty() const {
        return dirty;
      }

      size_t Count() const {
        return count;
      }

      // 0 is the newest entry
      Entry Get(size_t index) const;

      // "AA:BB:CC:DD:EE:FF", buffer must hold at least 18 bytes
      static void FormatAddress(const Entry& entry, char* buffer);
      // Short description of what the connection did
      static const char* Describe(const Entry& entry);

      // Called from the BLE stack. Return true when an alert should be raised.
      void OnConnect(const uint8_t* address, uint8_t addressType, bool knownBond, uint32_t time);
      bool OnPairingRequest();
      bool OnEncryptionChange(bool success, bool bonded);
      bool OnDisconnect(bool hasBond);

      // Called a few seconds after the connection is established
      bool Classify(bool hasBond);

    private:
      static constexpr uint32_t magic = 0x4c495356; // "VSIL"
      static constexpr uint8_t fileVersion = 1;
      static constexpr const char* fileName = "/intrusion.dat";

      struct FileData {
        uint32_t magic;
        uint8_t version;
        uint8_t enabled;
        uint8_t count;
        uint8_t head;
        std::array<Entry, MaxEntries> entries;
      };

      // Entry for the current connection: kept aside until it's classified, then committed to the log
      Entry& Active() {
        return committed ? entries[head] : current;
      }

      bool ClassifyPending(bool hasBond);
      void Commit();

      FS& fs;
      std::array<Entry, MaxEntries> entries {};
      uint8_t head = MaxEntries - 1;
      uint8_t count = 0;
      bool enabled = true;
      bool dirty = false;
      Entry current {};
      bool connected = false; // A connection is being tracked
      bool pending = false;   // The tracked connection isn't classified yet
      bool committed = false; // The tracked connection was written to the log
      TickType_t connectTick = 0;
    };
  }
}
