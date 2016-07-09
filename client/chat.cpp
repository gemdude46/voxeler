#define max_msgs 16

dimension2d<s32> charSize = dimension2d<s32>(16, 15);
SColor chatBG = SColor(128,0,0,0);
int lineHeight = -18, letterSpacing = -3;
#define chatPos position2d<s32>(32,SS.Height-64);

ITexture* font;

std::string chat[max_msgs];
clock_t lastChat = clock();

void writeChat(std::string msg) {
    range(i, 0, max_msgs-1) chat[i]=chat[i+1];
    chat[max_msgs-1]=msg;
    lastChat = clock();
}

void drawText(std::string text, position2d<s32> at) {
    position2d<s32> c = at;
    driver->draw2DRectangle(chatBG, rect<s32>(c, dimension2d<s32>((charSize.Width+letterSpacing)*text.length(),charSize.Height)));
    range(i,0,text.length()) {
        int ch = text.at(i);
        driver->draw2DImage(font, c, rect<s32>(dimension2d<s32>((ch&15)*charSize.
            Width,charSize.Height*(ch/16)),charSize), NULL, GREY(255), true);
        c.X += charSize.Width + letterSpacing;
    }
}

void drawChat() {
    if (((float)(clock()-lastChat))/CLOCKS_PER_SEC>3) return;
    position2d<s32> c = chatPos;
    for (int i = max_msgs-1; i > -1; i--) {
        drawText(chat[i], c);
        c.Y += lineHeight;
    }
}
