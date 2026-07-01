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

            // Check that the transport was initialized, and if not, do that now
            if(!this->m_transport->is_initialized())
            {
                result = this->m_transport->initialize();
                EPD_CHECK_ERR(result);
            }

            // First, allocate buffers.

            return EPD_OK;
        }

        error_t GDEY037T03::sleep()
        {
            return EPD_OK;
        }

        error_t GDEY037T03::refresh()
        {
            return EPD_OK;
        }
                
        error_t GDEY037T03::fill(color color)
        {
            return EPD_OK;
        }
    }
}