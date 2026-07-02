#pragma once

#include <array>
#include <cstddef>

#include "../epd_error.hxx"
#include "../epd_transport.hxx"


namespace epd
{
    namespace controllers
    {
        enum class UC8253_command
            : std::uint8_t
        {
            PSR = 0x00,
            PWR = 0x01,
            POF = 0x02,
            PFS = 0x03,
            PON = 0x04,
            PMES = 0x05,
            BTST = 0x06,
            DSLP = 0x07,
            DTM1 = 0x10,
            DSP = 0x11,
            DRF = 0x12,
            DTM2 = 0x13,
            AUTO = 0x17,
            LUTC = 0x20,
            LUTWW = 0x21,
            LUTKW = 0x22,
            LUTWK = 0x23,
            LUTKK = 0x24,
            LUTOPT = 0x2A,
            PLL = 0x30,
            TSC = 0x40,
            TSE = 0x41,
            TSW = 0x42,
            TSR = 0x43,
            PBC = 0x44,
            CDI = 0x50,
            LPD = 0x51,
            TCON = 0x60,
            TRES = 0x61,
            GSST = 0x65,
            REV = 0x70,
            FLG = 0x71,
            CRC = 0x72,
            AMV = 0x80,
            VV = 0x81,
            VDCS = 0x82,
            PTL = 0x90,
            PTIN = 0x91,
            PTOUT = 0x92,
            PGM = 0xA0,
            APG = 0xA1,
            ROTP = 0xA2,
            CCSET = 0xE0,
            PWS = 0xE3,
            LVSEL = 0xE4,
            TSSET = 0xE5
        };

        enum class UC8253_framebuffer
        {
            // BLACK color channel in KWR mode, OLD channel in KW mode
            BUFFER_1 = 0x0,

            // RED color channel in KWR mode, NEW channel in KW mode
            BUFFER_2 = 0x1
        };

        enum class UC8253_LUT_source
            : std::uint8_t
        {
            OTP = 0x00,
            REGISTER = 0x01
        };

        enum class UC8253_color_mode
            : std::uint8_t
        {
            // Black white red
            KWR = 0x00,

            // Black white
            KW = 0x01
        };

        enum class UC8253_scan_direction
            : std::uint8_t
        {
            SCAN_DOWN = 0x00,
            SCAN_UP = 0x01
        };

        enum class UC8253_shift_direction
            : std::uint8_t
        {
            SHIFT_LEFT = 0x00,
            SHIFT_RIGHT = 0x01
        };

        enum class UC8253_booster
            : std::uint8_t
        {
            BOOSTER_OFF = 0x00,
            BOOSTER_ON = 0x01
        };

        enum class UC8253_resolution
            : std::uint8_t
        {
            RES_240x120 = 0b00,
            RES_320x160 = 0b01,
            RES_400x200 = 0b10,
            RES_480x240 = 0b11
        };

        struct UC8253_config
        {
        protected:
            constexpr static std::uint8_t RESOLUTION_MASK = 0b11;
            constexpr static std::size_t RESOLUTION_SHIFT = 6UL;
            constexpr static std::uint8_t LUT_MASK = 0b1;
            constexpr static std::size_t LUT_SHIFT = 5UL;
            constexpr static std::uint8_t COLOR_MASK = 0b1;
            constexpr static std::size_t COLOR_SHIFT = 4UL;
            constexpr static std::uint8_t SCAN_MASK = 0b1;
            constexpr static std::size_t SCAN_SHIFT = 3UL;
            constexpr static std::uint8_t SHIFT_MASK = 0b1;
            constexpr static std::size_t SHIFT_SHIFT = 2UL;
            constexpr static std::uint8_t BOOSTER_MASK = 0b1;
            constexpr static std::size_t BOOSTER_SHIFT = 1UL;   

        public:
            UC8253_config() = default;

        public:
            std::array<std::uint8_t, 2> to_command_data() const;
            
        public:
            UC8253_resolution resolution{UC8253_resolution::RES_240x120};
            UC8253_LUT_source lut{UC8253_LUT_source::OTP};
            UC8253_color_mode color_mode{UC8253_color_mode::KWR};
            UC8253_scan_direction scan_direction{UC8253_scan_direction::SCAN_UP};
            UC8253_shift_direction shift_direction{UC8253_shift_direction::SHIFT_RIGHT};
            UC8253_booster booster{UC8253_booster::BOOSTER_ON};
            bool soft_reset{false};      
        };

        // Class for working with UC8253 ePaper controllers
        class UC8253
        {
            protected:
                // How long to wait in between BUSY line checks while waiting for an 
                // operation to finish
                constexpr static std::size_t BUSY_WAIT_DELAY = 50UL;

                // How long to wait in between RESET line level changes during
                // hardware reset sequence
                constexpr static std::size_t RESET_DELAY = 10UL;

                // Extra time to wait after powering off the panel
                constexpr static std::size_t POF_EXTRA_DELAY = 1000UL;

                // Special magic byte key to confirm intentional deep sleep command
                constexpr static std::uint8_t DEEP_SLEEP_KEY = 0xA5;

                // Extra time to wait after sending a display refresh command, before
                // checking the busy line
                constexpr static std::size_t DRF_EXTRA_DELAY = 10UL;

                // VCOM CDI setting for monochrome mode
                constexpr static std::size_t VCOM_CDI_MONOCHROME = 0x97;

            public:
                UC8253(transport* transport);

                UC8253(const UC8253&) = delete;
                UC8253& operator=(const UC8253&) = delete;

                UC8253(UC8253&&) = default;
                UC8253& operator=(UC8253&&) = default;

            public:
                error_t send_command(UC8253_command command);
                error_t send_command(UC8253_command command, std::size_t dataLength, const std::uint8_t* data);

                error_t power_up();
                error_t power_down();
                error_t wait_for_busy_pin();
                error_t hardware_reset();
                error_t deep_sleep();
                // Update display from internal framebuffer contents
                error_t update();
                error_t send_framebuffer(UC8253_framebuffer framebuffer, std::size_t dataLength, const std::uint8_t* data);
                UC8253_config* config();

            protected:
                error_t configure_monochrome();

            protected:
                transport* m_transport;
                UC8253_config m_config{ };
        };
    }
}