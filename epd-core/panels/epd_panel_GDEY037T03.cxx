#include <algorithm>

#include "../epd_error.hxx"
#include "../epd_panel.hxx"
#include "epd_panel_GDEY037T03.hxx"

namespace epd
{
    namespace panels
    {
        GDEY037T03::GDEY037T03(transport* transport)
            :   epd::panel(GDEY037T03::WIDTH, GDEY037T03::HEIGHT, transport),
                m_controller{transport}
        {

        }

        error_t GDEY037T03::initialize()
        {
            error_t result = EPD_OK;

            // == Setup controller configuration for monochrome panel
            auto* controllerConfig = this->m_controller.config();
            controllerConfig->color_mode = controllers::UC8253_color_mode::KW;
            controllerConfig->resolution = controllers::UC8253_resolution::RES_480x240;
            // ==

            // == Check that the transport was initialized, and if not, do that now
            if(!this->m_transport->is_initialized())
            {
                result = this->m_transport->initialize();
                EPD_CHECK_ERR(result);
            }
            // ==

            // == Allocate framebuffer
            // Determine framebuffer size. Make sure that height is always a multiple of 
            auto stride = this->m_height;
            if((this->m_height % 8) != 0)
            {
                stride += (8 - (stride % 8));
            }

            this->m_framebufferSize = (this->m_width * (stride / 8));

            std::uint8_t* framebuffer{nullptr};
            result = this->m_transport->allocate_buffer(this->m_framebufferSize, &framebuffer);
            EPD_CHECK_ERR(result);

            this->m_framebuffer = framebuffer_ptr(framebuffer,
                [this](std::uint8_t* buffer) -> void
                {
                    this->m_transport->free_buffer(buffer);
                }
            );
            // ==

            // == Power on and initialize display controller
            result = this->m_controller.power_up();
            EPD_CHECK_ERR(result);
            // ==

            // == Clear frame buffer with white
            result = this->fill(color::black);
            EPD_CHECK_ERR(result);
            // ==

            // == Perform full refresh
            result = this->refresh();
            EPD_CHECK_ERR(result);
            // ==

            // == Power down, we are done here
            // XXX This is already done by the call to ::refresh().
            /*result = this->m_controller.power_down();
            EPD_CHECK_ERR(result);*/
            // ==

            return EPD_OK;
        }

        error_t GDEY037T03::sleep()
        {
            return EPD_OK;
        }

        error_t GDEY037T03::refresh()
        {
            error_t result = EPD_OK;

            // == First, power up controller
            result = this->m_controller.power_up();
            EPD_CHECK_ERR(result);
            // ==

            // == Then, send the frame buffer. We are using the OLD and NEW buffers here,
            // and for a full refresh both should be the same (is that the case?)
            result = this->m_controller.send_framebuffer(
                controllers::UC8253_framebuffer::BUFFER_1,
                this->m_framebufferSize,
                this->m_framebuffer.get()
            );
            EPD_CHECK_ERR(result);

            result = this->m_controller.send_framebuffer(
                controllers::UC8253_framebuffer::BUFFER_2,
                this->m_framebufferSize,
                this->m_framebuffer.get()
            );
            EPD_CHECK_ERR(result);
            // ==

            // == Finally, perform a panel update from internal RAM.
            result = this->m_controller.update();
            EPD_CHECK_ERR(result);
            // ==

            // == Afterwards, make sure to power down controller in order to 
            // save energy.
            result = this->m_controller.power_down();
            EPD_CHECK_ERR(result);
            // ==

            return EPD_OK;
        }
                
        error_t GDEY037T03::fill(color color)
        {
            // Black/White buffer is inverted in monochrome mode
            const std::uint8_t fillValue = (color == color::black) ? 0x00 : 0xFF;

            std::fill(this->m_framebuffer.get(), this->m_framebuffer.get() + this->m_framebufferSize, fillValue);

            return EPD_OK;
        }
    }
}