#include <algorithm>
#include <cstddef>

#include "../epd_error.hxx"
#include "../epd_panel.hxx"
#include "epd_panel_GDEY037T03.hxx"

namespace epd
{
    namespace panels
    {
        GDEY037T03::GDEY037T03(epd::transport* transport)
            :   epd::panel(GDEY037T03::WIDTH, GDEY037T03::HEIGHT, transport),
                m_controller{transport}
        {

        }

        bool GDEY037T03::supports_partial_refresh() const
        {
            return true;
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

            // == Allocate framebuffers
            // Determine framebuffer size. Make sure that height is always a multiple of 
            auto stride = this->m_height;
            if((this->m_height % 8) != 0)
            {
                stride += (8 - (stride % 8));
            }

            this->m_framebufferSize = (this->m_width * (stride / 8));

            result = this->allocate_framebuffer(&this->m_oldFramebuffer, this->m_framebufferSize);
            EPD_CHECK_ERR(result);

            result = this->allocate_framebuffer(&this->m_newFramebuffer, this->m_framebufferSize);
            EPD_CHECK_ERR(result);
            // ==

            // == Power on and initialize display controller
            result = this->m_controller.power_up();
            EPD_CHECK_ERR(result);
            // ==

            // == Clear frame buffer with white
            result = this->fill(color::white);
            EPD_CHECK_ERR(result);
            // ==

            // == Perform full refresh
            result = this->refresh();
            EPD_CHECK_ERR(result);
            // ==

            return EPD_OK;
        }

        error_t GDEY037T03::sleep()
        {
            error_t result = EPD_OK;

            // We cant do deep sleep if we do not have a hardware reset line,
            // since we wouldnt be able to wake the controller back up!
            if(!this->m_transport->has_hw_reset())
            {
                return EPD_FAIL;
            }

            result = this->m_controller.deep_sleep();
            EPD_CHECK_ERR(result);

            return EPD_OK;
        }

        error_t GDEY037T03::refresh()
        {
            error_t result = EPD_OK;

            // == First, power up controller
            result = this->m_controller.power_up(controllers::UC8253_refresh_mode::FULL_REFRESH);
            EPD_CHECK_ERR(result);
            // ==

            // == Then, send the frame buffers.
            result = this->send_framebuffers();
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

        error_t GDEY037T03::partial_refresh(const rectangle& bounds)
        {
            error_t result{EPD_OK};

            // == First, power up controller
            result = this->m_controller.power_up(controllers::UC8253_refresh_mode::PARTIAL_REFRESH);
            EPD_CHECK_ERR(result);
            // ==

            // == Then, send the frame buffers.
            result = this->send_framebuffers();
            EPD_CHECK_ERR(result);
            // ==

            // == Finally, ask the controller to perform a partial update from RAM.
            result = this->m_controller.update_partial(bounds);
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

            std::fill(this->m_newFramebuffer.get(), this->m_newFramebuffer.get() + this->m_framebufferSize, fillValue);

            return EPD_OK;
        }

        error_t GDEY037T03::set_pixel(position pos, color color)
        {
            if(pos.x >= this->m_width || pos.y >= this->m_height)
            {
                return EPD_FAIL;
            }

            const auto index = ((pos.y * this->m_width) + pos.x) / 8;
            const std::size_t offset = 7UL - (pos.x & 0x07);
            const std::uint8_t colorBit = (color == color::black) ? 0b0 : 0b1;

            auto* framebuffer = this->m_newFramebuffer.get();

            framebuffer[index] = (framebuffer[index] & ~(0b1 << offset)) | (colorBit << offset);

            return EPD_OK;
        }

        error_t GDEY037T03::allocate_framebuffer(framebuffer_ptr* target, std::size_t framebufferSize)
        {
            if(!target)
            {
                return EPD_FAIL;
            }

            error_t result{EPD_OK};

            std::uint8_t* framebuffer{nullptr};
            result = this->m_transport->allocate_buffer(framebufferSize, &framebuffer);
            EPD_CHECK_ERR(result);

            *target = framebuffer_ptr(framebuffer,
                [this](std::uint8_t* buffer) -> void
                {
                    this->m_transport->free_buffer(buffer);
                }
            );

            std::fill(framebuffer, framebuffer + framebufferSize, 0x00);

            return EPD_OK;
        }

        error_t GDEY037T03::send_framebuffers()
        {
            error_t result{EPD_OK};

            // == First, write the old data. It resides in framebuffer 1.
            result = this->m_controller.send_framebuffer(
                controllers::UC8253_framebuffer::BUFFER_1,
                this->m_framebufferSize,
                this->m_oldFramebuffer.get()
            );
            EPD_CHECK_ERR(result);
            // ==

            // == Now, send the new data. It resides in framebuffer 2.
            result = this->m_controller.send_framebuffer(
                controllers::UC8253_framebuffer::BUFFER_2,
                this->m_framebufferSize,
                this->m_newFramebuffer.get()
            );
            EPD_CHECK_ERR(result);
            // ==

            // == The contents of the new framebuffer now have to become the
            // old framebuffer content.
            // XXX Do some kind of buffer switch using pointers so we dont have
            //     to copy the buffer contents here?
            std::copy(this->m_newFramebuffer.get(), this->m_newFramebuffer.get() + this->m_framebufferSize, this->m_oldFramebuffer.get());

            return EPD_OK;
        }
    }
}