#include <button.hpp>
#include <overlay.hpp>

void ld::Button::DrawTexture(const char* text, ::Texture2D texture, uint32_t spriteRow, uint32_t spriteCol, ::Color tint, bool isInteractable)
{
    if (isInteractable) {
        if (IsPressedDown()) {
            tint = ::Fade(::DARKBLUE, 0.8f);
        }
        else if (IsHovered()) {
            tint = ::Fade(Color{0, 255, 255, 255}, 0.5f);
        }
    }

    ::DrawTextureRec(
      texture
      , ::Rectangle {
        .x = static_cast<float>(spriteCol) * 32.0f,
        .y = static_cast<float>(spriteRow) * 32.0f,
        .width = 32.0f,
        .height = 32.0f,
      }
      , ::Vector2{
          static_cast<float>(xPos),
          static_cast<float>(yPos)
        }
      , tint
    );

    int fontSize = 10;
    int textWidth = ::MeasureText(text, fontSize);
    ld::DrawOutlinedText(text, xPos + 0.5f*(width - textWidth), yPos + 0.5f*(height-fontSize), 10, ::WHITE, ::BLACK);
}

void ld::Button::Draw(const char* text, uint32_t fontSize, ::Color color)
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
