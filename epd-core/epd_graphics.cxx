#include "epd_graphics.hxx"
#include "epd_error.hxx"
#include "epd_types.hxx"
#include <cstdlib>
#include <utility>

namespace epd
{
    graphics::graphics(panel* panel, epd::rotation rotation)
        : m_panel{panel}, m_transport{panel->transport()}
    {
        this->set_rotation(rotation);
    }

    void graphics::set_rotation(epd::rotation rotation)
    {
        // Adjust virtual width and height based on new rotation
        switch(rotation)
        {
            case epd::rotation::by_0deg:
            case epd::rotation::by_180deg:
                this->m_width = this->m_panel->width();
                this->m_height = this->m_panel->height();
                break;

            case epd::rotation::by_90deg:
            case epd::rotation::by_270deg:
                this->m_height = this->m_panel->width();
                this->m_width = this->m_panel->height();
                break;

            default:
                return;
        }

        this->m_rotation = rotation;
    }

    epd::rotation graphics::current_rotation() const
    {
        return this->m_rotation;
    }

    std::uint32_t graphics::width() const
    {
        return this->m_width;
    }

    std::uint32_t graphics::height() const
    {
        return this->m_height;
    }

    void graphics::clear(color color)
    {
        this->m_panel->fill(color);
    }

    error_t graphics::display()
    {
        error_t result{EPD_OK};

        result = this->m_panel->refresh();
        EPD_CHECK_ERR(result);

        result = this->m_panel->sleep();
        EPD_CHECK_ERR(result);

        return EPD_OK;
    }

    void graphics::draw_pixel(position position, color color)
    {
        // Perform bounds check to avoid out-of-bounds buffer access
        if(position.x >= this->width() || position.y >= this->height())
        {
            return;
        }

        // Rotate coordinates based on current rotation mode
        std::uint32_t temp{ };
        switch(this->m_rotation)
        {
            case epd::rotation::by_90deg:
                temp = position.x;
                position.x = this->m_panel->width() - 1 - position.y;
                position.y = temp;
                break;

            case epd::rotation::by_180deg:
                position.x = this->m_panel->width() - 1 - position.x;
                position.y = this->m_panel->height() - 1 - position.y;
                break;

            case epd::rotation::by_270deg:
                temp = position.x;
                position.x = position.y;
                position.y = this->m_panel->height() - 1 - temp;
                break;

            default:
                break;
        }

        // Write to panel framebuffer
        this->m_panel->set_pixel(position, color);
    }

    void graphics::draw_hline(position position, std::uint32_t width, color color)
    {
        epd::position to{ position.x + static_cast<std::int32_t>(width) - 1, position.y };
        this->draw_line(position, to, color);
    }

    void graphics::draw_vline(position position, std::uint32_t height, color color)
    {
        epd::position to{ position.x, position.y + static_cast<std::int32_t>(height) - 1 };
        this->draw_line(position, to, color);
    }

    void graphics::draw_line(position from, position to, color color)
    {
        this->m_transport->yield();

        const auto isSteep = abs(to.y - from.y) > abs(to.x - from.x);

        if(isSteep)
        {
            std::swap(from.x, from.y);
            std::swap(to.x, to.y);
        }

        if(from.x > to.x)
        {
            std::swap(from.x, to.x);
            std::swap(from.y, to.y);
        }

        std::int32_t dx{ to.x - from.x };
        std::int32_t dy{ abs(to.y - from.y) };

        std::int32_t error = dx / 2;
        std::int32_t yStep{ (from.y < to.y) ? 1 : -1 };

        for(; from.x <= to.x; from.x++)
        {
            if(isSteep)
            {
                this->draw_pixel(position{from.y, from.x}, color);
            }
            else
            {
                this->draw_pixel(from, color);
            }

            error -= dy;

            if(error < 0)
            {
                from.y += yStep;
                error += dx;
            }
        }
    }

    void graphics::draw_rect(position topLeft, std::uint32_t width, std::uint32_t height, color color)
    {
        this->draw_hline(topLeft, width, color);
        this->draw_hline(epd::position{topLeft.x, topLeft.y + (std::int32_t)height - 1}, width, color);

        this->draw_vline(topLeft, height, color);
        this->draw_vline(epd::position{topLeft.x + (std::int32_t)width - 1, topLeft.y}, height, color);
    }
}