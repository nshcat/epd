#pragma once

#include "../epd_panel.hxx"
#include <cstdint>
#include <memory>

namespace epd
{
    namespace panels
    {
        class GDEY037T03
            : public epd::panel
        {
            public:
                constexpr static std::size_t WIDTH = 240UL;
                constexpr static std::size_t HEIGHT = 416UL;

            public:
                GDEY037T03(transport* transport);

                virtual ~GDEY037T03() = default;

                GDEY037T03(const GDEY037T03&) = delete;
                GDEY037T03& operator=(const GDEY037T03&) = delete;

                GDEY037T03(GDEY037T03&&) = default;
                GDEY037T03& operator=(GDEY037T03&&) = default;

            public: // Panel management
                virtual error_t initialize() override;
                virtual error_t sleep() override;
                virtual error_t refresh() override;
                
            public: // Panel graphics operations
                virtual error_t fill(color color) override;

            protected:
                error_t setup_framebuffers();

            protected:
                std::size_t m_framebufferSize{ };
                std::unique_ptr<std::uint8_t> m_framebufferA{ };
                std::unique_ptr<std::uint8_t> m_framebufferB{ };
        };
    }
}