#pragma once

#include <cstdint>
#include <memory>
#include <functional>

#include "../epd_panel.hxx"
#include "../controllers/epd_controller_UC8253.hxx"

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

            protected:
                using framebuffer_ptr = std::unique_ptr<std::uint8_t, std::function<void(std::uint8_t*)>>;

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
                controllers::UC8253 m_controller;
                framebuffer_ptr m_framebuffer{ };
        };
    }
}