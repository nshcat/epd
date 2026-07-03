#include "epd_controller_UC8253.hxx"
#include <array>

namespace epd
{
    namespace controllers
    {
        std::array<std::uint8_t, 2> UC8253_config::to_command_data() const
        {
            // Build up command data payload based on high-level panel configuration.
            // The second byte is fixed to 0x8D for now, which is the default configuration,
            // because the configuration options contained within it aren't that interesting to
            // the vast majority of users. We should someday map those as well though, just
            // for completeness sake
            std::array<std::uint8_t, 2> data{ 0x00, 0x8D };

            // Panel resolution
            data[0] |= (static_cast<std::uint8_t>(this->resolution) & RESOLUTION_MASK) << RESOLUTION_SHIFT;

            // LUT selection
            data[0] |= (static_cast<std::uint8_t>(this->lut) & LUT_MASK) << LUT_SHIFT;

            // Color mode
            data[0] |= (static_cast<std::uint8_t>(this->color_mode) & COLOR_MASK) << COLOR_SHIFT;

            // Scan direction
            data[0] |= (static_cast<std::uint8_t>(this->scan_direction) & SCAN_MASK) << SCAN_SHIFT;    

            // Shift direction
            data[0] |= (static_cast<std::uint8_t>(this->shift_direction) & SHIFT_MASK) << SHIFT_SHIFT;   

            // Booster switch
            data[0] |= (static_cast<std::uint8_t>(this->booster) & BOOSTER_MASK) << BOOSTER_SHIFT; 

            // Soft reset
            if(!this->soft_reset)
            {
                // LSB is the soft reset flag. It is ACTIVE LOW.
                data[0] |= 0b1;
            }

            return data;
        }

        UC8253::UC8253(transport* transport)
            : m_transport{transport}
        {

        }

        UC8253_config* UC8253::config()
        {
            return &this->m_config;
        }

        error_t UC8253::send_command(UC8253_command command)
        {
            return this->send_command(command, 0, nullptr);
        }
        
        error_t UC8253::send_command(UC8253_command command, std::size_t dataLength, const std::uint8_t* data)
        {
            return this->m_transport->send_command(static_cast<std::uint8_t>(command), dataLength, data);
        }

        error_t UC8253::power_up(UC8253_refresh_mode refreshMode)
        {
            error_t result{EPD_OK};

            // == First, perform a hardware reset
            result = this->hardware_reset();
            EPD_CHECK_ERR(result);
            // ==

            // == Then, send the power on command. We have to wait for
            // the busy line to clear here, since power on might take
            // a while.
            result = this->send_command(UC8253_command::PON);
            EPD_CHECK_ERR(result);

            result = this->wait_for_busy_pin();
            EPD_CHECK_ERR(result);
            // ==

            // == If we are in monochrome mode, we need to reconfigure the VCOM
            // and data interval settings to reflect panel timing requirements
            // that differ from the default settings.
            /*if(this->m_config.color_mode == UC8253_color_mode::KW)
            {
                result = this->configure_monochrome();
                EPD_CHECK_ERR(result);
            }*/
            // ==

            // == Setup panel temperature sensor override.
            // For partial refresh mode, we force a faster panel refresh procedure
            // by faking a very high ambient temperature (since the ink particles move
            // more quickly if the enclosing liquid is warmer, this causes the controller
            // to be quicker with the refresh). This will accumulate artifacts over time though!
            if(refreshMode == UC8253_refresh_mode::PARTIAL_REFRESH)
            {
                result = this->set_temperature_source(UC8253_temperature_source::OVERRIDE);
                EPD_CHECK_ERR(result);

                result = this->set_temperature_override(UC8253::PARTIAL_REFRESH_TSSET);
                EPD_CHECK_ERR(result);

                // Also, set VCOM to floating for the border, otherwise we would mess up the pixels
                // surrounding our partial refresh region
                result = this->disable_border();
                EPD_CHECK_ERR(result);
            }
            else
            {
                // For full refreshs, we afford ourselves way more time to refresh the screen,
                // in order to ensure that no artifacts will appear.
                result = this->set_temperature_source(UC8253_temperature_source::OVERRIDE);
                EPD_CHECK_ERR(result);

                result = this->set_temperature_override(UC8253::FULL_REFRESH_TSSET);
                EPD_CHECK_ERR(result);
            }
            // ==

            // == Configure the panel. This will also assert the busy line,
            // so wait for it to clear.
            const auto panelConfig = this->m_config.to_command_data();
            result = this->send_command(UC8253_command::PSR, panelConfig.size(), panelConfig.data());
            EPD_CHECK_ERR(result);

            result = this->wait_for_busy_pin();
            EPD_CHECK_ERR(result);
            // ==

            return EPD_OK;
        }

        error_t UC8253::power_down()
        {
            error_t result{EPD_OK};

            // == Send power off command. This will also assert the busy line - wait
            // for it to clear. We also are advised by the data sheet to wait a second
            // here - just to really give the controller enough time to power down.
            result = this->send_command(UC8253_command::POF);
            EPD_CHECK_ERR(result);

            result = this->wait_for_busy_pin();
            EPD_CHECK_ERR(result);

            result = this->m_transport->delay(UC8253::POF_EXTRA_DELAY);
            EPD_CHECK_ERR(result);
            // ==

            // == Now, put the panel into deep sleep. This will increase the life time of the
            // panel considerably.
            result = this->deep_sleep();
            EPD_CHECK_ERR(result);
            // ==

            return EPD_OK;
        }

        /*error_t UC8253::configure_monochrome()
        {
            error_t result{EPD_OK};

            std::array<std::uint8_t, 1> commandPayload{ VCOM_CDI_MONOCHROME };
            result = this->send_command(UC8253_command::CDI, commandPayload.size(), commandPayload.data());
            EPD_CHECK_ERR(result);

            return EPD_OK;
        }*/

        error_t UC8253::disable_border()
        {
            error_t result{EPD_OK};

            std::array<std::uint8_t, 1> commandPayload{ VCOM_CDI_PARTIAL };
            result = this->send_command(UC8253_command::CDI, commandPayload.size(), commandPayload.data());
            EPD_CHECK_ERR(result);

            return EPD_OK;
        }

        error_t UC8253::set_temperature_source(UC8253_temperature_source tempSource)
        {
            const std::uint8_t ccsetConfig
                = (static_cast<std::uint8_t>(tempSource) & UC8253::CCSET_TSFIX_MASK) << UC8253::CCSET_TSFIX_SHIFT;
               
            return this->send_command(UC8253_command::CCSET, 1UL, &ccsetConfig);
        }


        error_t UC8253::set_temperature_override(std::uint8_t temperature)
        {
            std::array<std::uint8_t, 1> commandPayload{ temperature };

            return this->send_command(UC8253_command::TSSET, commandPayload.size(), commandPayload.data());
        }

        error_t UC8253::deep_sleep()
        {
            error_t result{EPD_OK};

            // The deep sleep command expects a magic byte as argument to confirm
            // that entering sleep is intentional.
            std::array<std::uint8_t, 1> commandPayload{ UC8253::DEEP_SLEEP_KEY };

            result = this->send_command(UC8253_command::DSLP, commandPayload.size(), commandPayload.data());
            EPD_CHECK_ERR(result);

            return EPD_OK;
        }

        error_t UC8253::update()
        {
            error_t result{EPD_OK};

            // Send display refresh command
            result = this->send_command(UC8253_command::DRF);
            EPD_CHECK_ERR(result);

            // To be safe, wait a little bit before checking busy line.
            // Otherwise, the controller might not have had enough time to assert it.
            result = this->m_transport->delay(UC8253::DRF_EXTRA_DELAY);
            EPD_CHECK_ERR(result);

            // Now, wait for the controller to signal that it is no longer busy.
            result = this->wait_for_busy_pin();
            EPD_CHECK_ERR(result);

            return EPD_OK;
        }

        error_t UC8253::update_partial(const rectangle& bounds)
        {
            error_t result{EPD_OK};

            // Force panel into partial refresh mode
            result = this->send_command(UC8253_command::PTIN);
            EPD_CHECK_ERR(result);

            // Setup partial refresh window
            result = this->setup_refresh_window(bounds);
            EPD_CHECK_ERR(result);

            // Send display refresh command
            result = this->send_command(UC8253_command::DRF);
            EPD_CHECK_ERR(result);

            // To be safe, wait a little bit before checking busy line.
            // Otherwise, the controller might not have had enough time to assert it.
            result = this->m_transport->delay(UC8253::DRF_EXTRA_DELAY);
            EPD_CHECK_ERR(result);

            // Now, wait for the controller to signal that it is no longer busy.
            result = this->wait_for_busy_pin();
            EPD_CHECK_ERR(result);

            return EPD_OK;
        }

        error_t UC8253::setup_refresh_window(const rectangle& bounds)
        {
            error_t result{EPD_OK};

            std::array<std::uint8_t, 7> commandData{};
            const auto topLeft = bounds.location;
            const auto bottomRight = bounds.bottom_right();

            // x start
            commandData[0] = topLeft.x;
            // x end
            commandData[1] = bottomRight.x;
            // y start, larger than 8 bits
            commandData[2] =  topLeft.y / 256;
            commandData[3] =  topLeft.y % 256;
            // y end, larger than 8 bits
            commandData[4] =  bottomRight.y / 256;
            commandData[5] =  bottomRight.y % 256;
            // End marker
            commandData[6] = 0x01;

            result = this->send_command(UC8253_command::PTL, commandData.size(), commandData.data());
            EPD_CHECK_ERR(result);

            return EPD_OK;
        }

        error_t UC8253::send_framebuffer(UC8253_framebuffer framebuffer, std::size_t dataLength, const std::uint8_t* data)
        {
            error_t result{EPD_OK};

            // Decide which command needs to be sent - DTM1 is for framebuffer 1,
            // DTM2 is for framebuffer 2
            const auto command =
                (framebuffer == UC8253_framebuffer::BUFFER_1 ? UC8253_command::DTM1 : UC8253_command::DTM2);

            // Send over buffer.
            result = this->send_command(command, dataLength, data);
            EPD_CHECK_ERR(result);

            return EPD_OK;
        }

        error_t UC8253::wait_for_busy_pin()
        {
            if(!this->m_transport->has_busy_pin())
            {
                return EPD_FAIL;
            }

            error_t result{EPD_OK};

            // Wait for BUSY pin to go HIGH, signalling end of operation
            while(true)
            {
                pin_state busyPinState{pin_state::low};
                result = this->m_transport->get_busy_pin(&busyPinState);
                EPD_CHECK_ERR(result);

                if(busyPinState == pin_state::high)
                {
                    return EPD_OK;
                }
                else
                {
                    result = this->send_command(UC8253_command::FLG);
                    EPD_CHECK_ERR(result);

                    result = this->m_transport->delay(UC8253::BUSY_WAIT_DELAY);
                    EPD_CHECK_ERR(result);
                }
            }

            return EPD_OK;
        }

        error_t UC8253::hardware_reset()
        {
            if(!this->m_transport->has_hw_reset())
            {
                return EPD_FAIL;
            }

            error_t result{EPD_OK};

            // First, go HIGH and wait a bit for voltage to settle
            result = this->m_transport->set_reset_pin(pin_state::high);
            EPD_CHECK_ERR(result);

            result = this->m_transport->delay(UC8253::RESET_DELAY);
            EPD_CHECK_ERR(result);

            // Then, pull the reset line LOW to put the controller into reset
            result = this->m_transport->set_reset_pin(pin_state::low);
            EPD_CHECK_ERR(result);

            result = this->m_transport->delay(UC8253::RESET_DELAY);
            EPD_CHECK_ERR(result);

            // Finally, pull reset line HIGH again to let controller out of reset
            result = this->m_transport->set_reset_pin(pin_state::high);
            EPD_CHECK_ERR(result);

            result = this->m_transport->delay(UC8253::RESET_DELAY);
            EPD_CHECK_ERR(result);

            return EPD_OK;
        }
    }
}