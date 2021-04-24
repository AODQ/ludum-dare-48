#include <button.hpp>

void ld::Button::Draw(const char* text)
{
    int textWidth = ::MeasureText(text, fontSize);
    ::DrawRectangleLines(xPos, yPos, width, height, color);
    ::DrawText(text, xPos + 0.5f*(width - textWidth), yPos + 0.5f*(height-fontSize), fontSize, BLACK);
}

bool ld::Button::IsClicked()
{
    auto mousePos = ::GetMousePosition();
    if (::CheckCollisionPointRec(mousePos, bounds))
    {
        if (::IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            return true;
        }
    }

    return false;
}
