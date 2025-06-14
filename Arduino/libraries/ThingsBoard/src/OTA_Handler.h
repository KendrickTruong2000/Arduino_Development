/*
  OTA_Handler.h - Library API for sending data to the ThingsBoard
  Based on PubSub MQTT library.
  Created by Olender M. Oct 2018.
  Released into the public domain.
*/
#ifndef OTA_Handler_h
#define OTA_Handler_h

// Local include.
#include "Configuration.h"

#if THINGSBOARD_ENABLE_OTA

// Local include.
#include "Callback_Watchdog.h"
#include "HashGenerator.h"
#include "Helper.h"
#include "OTA_Update_Callback.h"
#include "OTA_Failure_Response.h"


/// ---------------------------------
/// Constant strings in flash memory.
/// ---------------------------------
// Firmware data keys.
#if THINGSBOARD_ENABLE_PROGMEM
constexpr char FW_STATE_DOWNLOADING[] PROGMEM = "DOWNLOADING";
constexpr char FW_STATE_DOWNLOADED[] PROGMEM = "DOWNLOADED";
constexpr char FW_STATE_VERIFIED[] PROGMEM = "VERIFIED";
constexpr char FW_STATE_UPDATING[] PROGMEM = "UPDATING";
constexpr char FW_STATE_UPDATED[] PROGMEM = "UPDATED";
constexpr char FW_STATE_FAILED[] PROGMEM = "FAILED";
#else
constexpr char FW_STATE_DOWNLOADING[] = "DOWNLOADING";
constexpr char FW_STATE_DOWNLOADED[] = "DOWNLOADED";
constexpr char FW_STATE_VERIFIED[] = "VERIFIED";
constexpr char FW_STATE_UPDATING[] = "UPDATING";
constexpr char FW_STATE_UPDATED[] = "UPDATED";
constexpr char FW_STATE_FAILED[] = "FAILED";
#endif // THINGSBOARD_ENABLE_PROGMEM

// Log messages.
#if THINGSBOARD_ENABLE_PROGMEM
constexpr char UNABLE_TO_REQUEST_CHUNCKS[] PROGMEM = "Unable to request firmware chunk";
constexpr char RECEIVED_UNEXPECTED_CHUNK[] PROGMEM = "Received chunk (%u), not the same as requested chunk (%u)";
constexpr char ERROR_UPDATE_BEGIN[] PROGMEM = "Failed to initalize flash updater";
constexpr char ERROR_UPDATE_WRITE[] PROGMEM = "Only wrote (%u) bytes of binary data to flash memory instead of expected (%u)";
constexpr char UPDATING_HASH_FAILED[] PROGMEM = "Updating hash failed";
constexpr char ERROR_UPDATE_END[] PROGMEM = "Error (%u) during flash updater not all bytes written";
constexpr char CHKS_VER_FAILED[] PROGMEM = "Checksum verification failed";
constexpr char FW_CHUNK[] PROGMEM = "Receive chunk (%i), with size (%u) bytes";
constexpr char HASH_ACTUAL[] PROGMEM = "(%s) actual checksum: (%s)";
constexpr char HASH_EXPECTED[] PROGMEM = "(%s) expected checksum: (%s)";
constexpr char CHKS_VER_SUCCESS[] PROGMEM = "Checksum is the same as expected";
constexpr char FW_UPDATE_ABORTED[] PROGMEM = "Firmware update aborted";
constexpr char FW_UPDATE_SUCCESS[] PROGMEM = "Update success";
#else
constexpr char UNABLE_TO_REQUEST_CHUNCKS[] = "Unable to request firmware chunk";
constexpr char RECEIVED_UNEXPECTED_CHUNK[] = "Received chunk (%u), not the same as requested chunk (%u)";
constexpr char ERROR_UPDATE_BEGIN[] = "Failed to initalize flash updater";
constexpr char ERROR_UPDATE_WRITE[] = "Only wrote (%u) bytes of binary data to flash memory instead of expected (%u)";
constexpr char UPDATING_HASH_FAILED[] = "Updating hash failed";
constexpr char ERROR_UPDATE_END[] = "Error during flash updater not all bytes written";
constexpr char CHKS_VER_FAILED[] = "Checksum verification failed";
constexpr char FW_CHUNK[] = "Receive chunk (%i), with size (%u) bytes";
constexpr char HASH_ACTUAL[] = "(%s) actual checksum: (%s)";
constexpr char HASH_EXPECTED[] = "(%s) expected checksum: (%s)";
constexpr char CHKS_VER_SUCCESS[] = "Checksum is the same as expected";
constexpr char FW_UPDATE_ABORTED[] = "Firmware update aborted";
constexpr char FW_UPDATE_SUCCESS[] = "Update success";
#endif // THINGSBOARD_ENABLE_PROGMEM


/// @brief Handles actually writing the received firmware packets into flash memory
/// @tparam Logger Logging class that should be used to print messages generated by ThingsBoard
template<typename Logger>
class OTA_Handler {
  public:
    /// @brief Constructor
    /// @param publish_callback Callback that is used to request the firmware chunk of the firmware binary with the given chunk number
    /// @param send_fw_state_callback Callback that is used to send information about the current state of the over the air update
    /// @param finish_callback Callback that is called once the update has been finished and the user has been informed of the failure or success
    inline OTA_Handler(std::function<bool(const uint32_t&)> publish_callback, std::function<bool(const char *, const char *)> send_fw_state_callback, std::function<bool(void)> finish_callback)
        : m_fw_callback(nullptr)
        , m_publish_callback(publish_callback)
        , m_send_fw_state_callback(send_fw_state_callback)
        , m_finish_callback(finish_callback)
        , m_fw_size(0U)
        , m_fw_algorithm()
        , m_fw_checksum()
        , m_fw_checksum_algorithm()
        , m_hash()
        , m_total_chunks(0U)
        , m_requested_chunks(0U)
        , m_retries(0U)
        , m_watchdog(std::bind(&OTA_Handler::Handle_Request_Timeout, this))
    {
      // Nothing to do
    }

    /// @brief Starts the firmware update with requesting the first firmware packet and initalizes the underlying needed components
    /// @param fw_callback Callback method that contains configuration information, about the over the air update
    /// @param fw_size Complete size of the firmware binary that will be downloaded and flashed onto this device
    /// @param fw_algorithm String of the algorithm used to hash the firmware binary
    /// @param fw_checksum Checksum of the complete firmware binary, should be the same as the actually written data in the end
    /// @param fw_checksum_algorithm Algorithm used to hash the firmware binary
    inline void Start_Firmware_Update(const OTA_Update_Callback *fw_callback, const uint32_t& fw_size, const std::string& fw_algorithm, const std::string& fw_checksum, const mbedtls_md_type_t& fw_checksum_algorithm) {
        m_fw_callback = fw_callback;
        m_fw_size = fw_size;
        m_total_chunks = (m_fw_size / m_fw_callback->Get_Chunk_Size()) + 1U;
        m_fw_algorithm = fw_algorithm;
        m_fw_checksum = fw_checksum;
        m_fw_checksum_algorithm = fw_checksum_algorithm;
        m_fw_updater = m_fw_callback->Get_Updater();

        if (!m_publish_callback || !m_send_fw_state_callback || !m_finish_callback || !m_fw_updater) {
          Logger::log(OTA_CB_IS_NULL);
          (void)m_send_fw_state_callback(FW_STATE_FAILED, OTA_CB_IS_NULL);
            return Handle_Failure(OTA_Failure_Response::RETRY_NOTHING);
        }
        Request_First_Firmware_Packet();
    }

    /// @brief Stops the firmware update
    inline void Stop_Firmware_Update() {
        m_watchdog.detach();
        m_fw_updater->reset();
        Logger::log(FW_UPDATE_ABORTED);
        (void)m_send_fw_state_callback(FW_STATE_FAILED, FW_UPDATE_ABORTED);
        Handle_Failure(OTA_Failure_Response::RETRY_NOTHING);
        m_fw_callback = nullptr;
    }

    /// @brief Uses the given firmware packet data and process it. Starting with writing the given amount of bytes of the packet data into flash memory and
    /// into a hash function that will be used to compare the expected complete binary file and the actually downloaded binary file
    /// @param current_chunk Index of the chunk we recieved the binary data for
    /// @param payload Firmware packet data of the current chunk
    /// @param total_bytes Amount of bytes in the current firmware packet data
    inline void Process_Firmware_Packet(const uint32_t& current_chunk, uint8_t *payload, const unsigned int& total_bytes) {
        (void)m_send_fw_state_callback(FW_STATE_DOWNLOADING, nullptr);

        if (current_chunk != m_requested_chunks) {
          char message[Helper::detectSize(RECEIVED_UNEXPECTED_CHUNK, current_chunk, m_requested_chunks)];
          snprintf_P(message, sizeof(message), RECEIVED_UNEXPECTED_CHUNK, current_chunk, m_requested_chunks);
          Logger::log(message);
          return;
        }

        m_watchdog.detach();

        char message[Helper::detectSize(FW_CHUNK, current_chunk, total_bytes)];
        snprintf_P(message, sizeof(message), FW_CHUNK, current_chunk, total_bytes);
        Logger::log(message);

        if (current_chunk == 0U) {
            // Initialize Flash
            if (!m_fw_updater->begin(m_fw_size)) {
              Logger::log(ERROR_UPDATE_BEGIN);
              (void)m_send_fw_state_callback(FW_STATE_FAILED, ERROR_UPDATE_BEGIN);
              return Handle_Failure(OTA_Failure_Response::RETRY_UPDATE);
            }
        }

        // Write received binary data to flash partition
        const size_t written_bytes = m_fw_updater->write(payload, total_bytes);
        if (written_bytes != total_bytes) {
            char message[Helper::detectSize(ERROR_UPDATE_WRITE, written_bytes, total_bytes)];
            snprintf_P(message, sizeof(message), ERROR_UPDATE_WRITE, written_bytes, total_bytes);
            Logger::log(message);
            (void)m_send_fw_state_callback(FW_STATE_FAILED, message);
            return Handle_Failure(OTA_Failure_Response::RETRY_UPDATE);
        }

        // Update value only if writing to flash was a success
        if (!m_hash.update(payload, total_bytes)) {
            Logger::log(UPDATING_HASH_FAILED);
            (void)m_send_fw_state_callback(FW_STATE_FAILED, UPDATING_HASH_FAILED);
            return Handle_Failure(OTA_Failure_Response::RETRY_UPDATE);
        }

        m_requested_chunks = current_chunk + 1;
        m_fw_callback->Call_Progress_Callback<Logger>(m_requested_chunks, m_total_chunks);

        // Ensure to check if the update was cancelled during the progress callback,
        // if it was the callback variable was reset and there is no need to request the next firmware packet
        if (m_fw_callback == nullptr) {
          return;
        }

        // Reset retries as the current chunk has been downloaded and handled successfully
        m_retries = m_fw_callback->Get_Chunk_Retries();
        Request_Next_Firmware_Packet();
    }

  private:
    const OTA_Update_Callback *m_fw_callback;
    std::function<bool(const uint32_t&)> m_publish_callback;
    std::function<bool(const char *, const char *)> m_send_fw_state_callback;
    std::function<bool(void)> m_finish_callback;
    // Allows for a binary size of up to theoretically 4 GB.
    size_t m_fw_size;
    std::string m_fw_algorithm;
    std::string m_fw_checksum;
    mbedtls_md_type_t m_fw_checksum_algorithm;
    IUpdater *m_fw_updater;
    HashGenerator m_hash;
    uint32_t m_total_chunks;
    uint32_t m_requested_chunks;
    uint8_t m_retries;
    Callback_Watchdog m_watchdog;

    inline void Request_First_Firmware_Packet() {
        m_requested_chunks = 0U;
        m_retries = m_fw_callback->Get_Chunk_Retries();
        m_hash.start(m_fw_checksum_algorithm);
        m_watchdog.detach();
        m_fw_updater->reset();
        Request_Next_Firmware_Packet();
    }

    inline void Request_Next_Firmware_Packet() {
        // Check if we have already requested and handled the last remaining chunk
        if (m_requested_chunks >= m_total_chunks) {
            Finish_Firmware_Update();   
            return;
        }

        if (!m_publish_callback(m_requested_chunks)) {
          Logger::log(UNABLE_TO_REQUEST_CHUNCKS);
          (void)m_send_fw_state_callback(FW_STATE_FAILED, UNABLE_TO_REQUEST_CHUNCKS);
        }

        // Watchdog gets started no matter if publishing request was successful or not in hopes,
        // that after the given timeout the callback calls this method again and can then publish the request successfully.
        m_watchdog.once(m_fw_callback->Get_Timeout());
    }

    inline void Finish_Firmware_Update() {
        (void)m_send_fw_state_callback(FW_STATE_DOWNLOADED, nullptr);

        const std::string calculated_hash = m_hash.get_hash_string();
        char actual[JSON_STRING_SIZE(strlen(HASH_ACTUAL)) + JSON_STRING_SIZE(m_fw_algorithm.size()) + JSON_STRING_SIZE(calculated_hash.size())];
        snprintf_P(actual, sizeof(actual), HASH_ACTUAL, m_fw_algorithm.c_str(), calculated_hash.c_str());
        Logger::log(actual);

        char expected[JSON_STRING_SIZE(strlen(HASH_EXPECTED)) + JSON_STRING_SIZE(m_fw_algorithm.size()) + JSON_STRING_SIZE(m_fw_checksum.size())];
        snprintf_P(expected, sizeof(expected), HASH_EXPECTED, m_fw_algorithm.c_str(), m_fw_checksum.c_str());
        Logger::log(expected);

        // Check if the initally received checksum is the same as the one we calculated from the received binary data,
        // if not we assume the binary data has been changed or not completly downloaded --> Firmware update failed
        if (m_fw_checksum.compare(calculated_hash) != 0) {
            Logger::log(CHKS_VER_FAILED);
            (void)m_send_fw_state_callback(FW_STATE_FAILED, CHKS_VER_FAILED);
            return Handle_Failure(OTA_Failure_Response::RETRY_UPDATE);
        }

        Logger::log(CHKS_VER_SUCCESS);

        if (!m_fw_updater->end()) {
            Logger::log(ERROR_UPDATE_END);
            (void)m_send_fw_state_callback(FW_STATE_FAILED, ERROR_UPDATE_END);
            return Handle_Failure(OTA_Failure_Response::RETRY_UPDATE);
        }

        Logger::log(FW_UPDATE_SUCCESS);
        (void)m_send_fw_state_callback(FW_STATE_UPDATING, nullptr);

        m_fw_callback->Call_Callback<Logger>(true);
        (void)m_finish_callback();
    }

    inline void Handle_Failure(const OTA_Failure_Response& failure_response) {
      if (m_retries <= 0) {
          m_fw_callback->Call_Callback<Logger>(false);
          (void)m_finish_callback();
          return;
      }

      // Decrease the amount of retries of downloads for the current chunk,
      // reset as soon as the next chunk has been received and handled successfully
      m_retries--;

      switch (failure_response) {
        case OTA_Failure_Response::RETRY_CHUNK:
          Request_Next_Firmware_Packet();
          break;
        case OTA_Failure_Response::RETRY_UPDATE:
          Request_First_Firmware_Packet();
          break;
        case OTA_Failure_Response::RETRY_NOTHING:
          m_fw_callback->Call_Callback<Logger>(false);
          (void)m_finish_callback();
          break;
        default:
          // Nothing to do
          break;
      }
    }

    inline void Handle_Request_Timeout() {
        return Handle_Failure(OTA_Failure_Response::RETRY_CHUNK);
    }
};

#endif // THINGSBOARD_ENABLE_OTA

#endif // OTA_Handler_h
