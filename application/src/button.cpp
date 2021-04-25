#include <button.hpp>
#include <overlay.hpp>

void ld::Button::Draw(const char* text, uint32_t fontSize)
{
    int textWidth = ::MeasureText(text, fontSize);
    ::DrawRectangleLines(xPos, yPos, width, height, ::DARKGRAY);

    if (IsPressedDown()) {
        ::DrawRectangle(xPos, yPos, width, height, ::Fade(::DARKBLUE, 0.8f));
    }
    else if (IsHovered()) {
        ::DrawRectangle(xPos, yPos, width, height, ::Fade(Color{0, 255, 255, 255}, 0.5f));
    }
    else {
        ::DrawRectangle(xPos, yPos, width, height, color);
    }

    ld::DrawOutlinedText(text, xPos + 0.5f*(width - textWidth), yPos + 0.5f*(height-fontSize), fontSize, ::WHITE, ::BLACK);
}

bool ld::Button::IsHovered()
{
    auto mousePos = ::GetMousePosition();
    if (::CheckCollisionPointRec(mousePos, bounds))
    {
        return true;
    }
    return false;
}

bool ld::Button::IsPressedDown()
{
    auto mousePos = ::GetMousePosition();
    if (::CheckCollisionPointRec(mousePos, bounds))
    {
        if (::IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            return true;
        }
    }
    return false;
}

bool ld::Button::IsClicked()
{
    auto mousePos = ::GetMousePosition();
    if (::CheckCollisionPointRec(mousePos, bounds))
    {
        //if (::IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (::IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            return true;
        }
    }

    return false;
}
