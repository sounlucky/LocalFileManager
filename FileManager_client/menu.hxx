template <uint16_t W, uint16_t H>
menuPointer Menu<W,H>::GetInput() {
    //if menu has changed there's a chance of getting out of range
    if (CurrMenu().size() < Pointer.pos)
        for (uint16_t i = 0; i < CurrMenu().size(); i++)
            if (ChooseAble(CurrMenu()[i]))
            {
                Pointer.pos = i;
                break;
            }
    Draw();
    unsigned char Input;
    while ((Input = GetCh()) != KEY_ENTER)
    {
        switch (Input)
        {
            case KEY_UP:
                for (uint16_t i = (uint16_t) (Pointer.pos == 0 ? CurrMenu().size() - 1 : (int8_t) Pointer.pos -
                                                                                         1 % CurrMenu().size());
                     i != Pointer.pos;
                     i = (uint16_t) (i == 0 ? CurrMenu().size() - 1 : (int8_t) i - 1 % CurrMenu().size()))
                {
                    if (ChooseAble(CurrMenu()[i])) {
                        Pointer.pos = i;
                        break;
                    }
                }
                break;
            case KEY_DOWN:
                for (uint16_t i = (uint16_t) ((Pointer.pos + 1) % CurrMenu().size());
                     i != Pointer.pos; i = (uint16_t) (
                        (i + 1) % CurrMenu().size())) {
                    if (ChooseAble(CurrMenu()[i])) {
                        Pointer.pos = i;
                        break;
                    }
                }
                break;
            case KEY_TAB:
                for (uint16_t i = 0; i < OthrMenu().size(); i++)
                    if (ChooseAble(OthrMenu().at(i))) {
                        Pointer.InMenu = !Pointer.InMenu;
                        Pointer.pos = i;
                        break;
                    }
            default:
                //probably Input
                if (InputAble(CurrMenu()[Pointer.pos]))
                {
                    if (('0' <= Input && Input <= '9') || ('A' <= Input && Input <= 'Z') ||
                        ('a' <= Input && Input <= 'z') || (Input == '.')) {
                        CurrMenu()[Pointer.pos] += Input;
                    } else if (Input == KEY_DELETE && CurrMenu()[Pointer.pos].length() > 1) {//delete
                        CurrMenu()[Pointer.pos].pop_back();
                    }
                }
        }
        Draw();
    }
    return Pointer;
}

template <uint16_t W, uint16_t H>
Menu<W,H>::Menu(std::vector<std::string> &InMenu, std::vector<std::string> &inContent) :
m_Linked_Menu(InMenu), m_Linked_Content(inContent) {
    std::string Border(W, '_'),
            Inside(W, ' ');
    Inside[0] = '|';
    Inside[W - 1] = '|';
//filling first "layer"
    m_ActualField.push_back(Border);
//middle
    int CurrMenuPos = 0, CurrContentPos = 0;
    for (uint16_t i = 0; i < H - 2; i++) {
        m_ActualField.push_back(Inside);
        if (i % 2) m_ActualField[i][W / 3] = '|';
    }
//last
    Border[0] = '|';
    Border[W - 1] = '|';
    m_ActualField.push_back(Border);
//create Pointer
    Pointer = {1, 0};
    for (uint16_t i = 0; i < InMenu.size(); i++)
        if (ChooseAble(InMenu[i]))
        {
            Pointer.pos = i;
            break;
        }
}

template <uint16_t W, uint16_t H>
void Menu<W,H>::Draw() {
    system("clear");
    Update();
    for (size_t i = 0; i < H; i++)
    {
        for (size_t g = 0; g < W; g++)
            std::cout << m_ActualField[i][g];
        std::cout << '\n';
    }
}

template <uint16_t W, uint16_t H>
u_char Menu<W,H>::GetCh() {
    u_char Buf = 0;
    struct termios Old = {0};
    fflush(stdout);
    if (tcgetattr(0, &Old) < 0)
        perror("tcsetattr()");
    Old.c_lflag &= ~ICANON;
    Old.c_lflag &= ~ECHO;
    Old.c_cc[VMIN] = 1;
    Old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &Old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &Buf, 1) < 0)
        perror("read()");
    Old.c_lflag |= ICANON;
    Old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &Old) < 0)
        perror("tcsetattr ~ICANON");
    return Buf;
}

template <uint16_t W, uint16_t H>
void Menu<W,H>::Update() {
    for (uint16_t i = 1; i < H - 1; i++)
        for (uint16_t j = 1; j < W - 1; j++)
            if ((i % 2) && j == W / 3)
                m_ActualField[i][j] = '|';
            else m_ActualField[i][j] = ' ';
    //Menu
    uint16_t Iterator = AboveMenu();
    for (uint16_t i = 1;//drawing from 1
         Iterator < ToBeDrawn(m_Linked_Menu, AboveMenu()) + AboveMenu();
         i += H / (ToBeDrawn(m_Linked_Menu, AboveMenu()) + 1))
    {
        if (Pointer.InMenu && Iterator == Pointer.pos)
        {
            m_ActualField[i][1] = '>';
        }
        for (uint16_t g = 0;
             g < RawStr(m_Linked_Menu[Iterator]).length() && g < W / 3 - 2;
                g++)
        {
            m_ActualField[i][g + 2] = RawStr(m_Linked_Menu[Iterator])[g];
        }
        Iterator++;
    }
    //Content
    Iterator = AboveContent();
    for (uint16_t i = 1;
         Iterator < ToBeDrawn(m_Linked_Content, AboveContent()) + AboveContent();
         i += H / (ToBeDrawn(m_Linked_Content, AboveContent()) + 1)) {
        if (!Pointer.InMenu && Iterator == Pointer.pos) {
            m_ActualField[i][W / 3 + 1] = '>';
        }
        for (uint16_t g = 0;
             g < RawStr(m_Linked_Content[Iterator]).length() && g < 2 * W / 3 - 2;
             g++) {
            m_ActualField[i][g + W / 3 + 2] = RawStr(m_Linked_Content[Iterator])[g];
        }
        Iterator++;
    }
}

template <uint16_t W, uint16_t H>
uint16_t Menu<W,H>::AboveTheCurrent() {
    return (uint16_t) (Pointer.pos / (H - 2) * (H - 2));
}

template <uint16_t W, uint16_t H>
uint16_t Menu<W,H>::AboveMenu() {
    return (uint16_t) (Pointer.InMenu ? AboveTheCurrent() : 0);
}

template <uint16_t W, uint16_t H>
uint16_t Menu<W,H>::AboveContent() {
    return (uint16_t) (!Pointer.InMenu ? AboveTheCurrent() : 0);
}

template <uint16_t W, uint16_t H>
uint16_t Menu<W,H>::ToBeDrawn(std::vector<std::string> &in, uint16_t Above) {
    return (uint16_t) ((uint16_t) in.size() - Above > H - 2 ?
       (unsigned long) (H - 2) : in.size() - Above);
}

template <uint16_t W, uint16_t H>
bool Menu<W,H>::ChooseAble(std::string In) {
    return In[0] != '~';
}

template <uint16_t W, uint16_t H>
bool Menu<W,H>::InputAble(std::string In) {
    return In[0] == '>';
}

template <uint16_t W, uint16_t H>
std::vector<std::string> &Menu<W,H>::CurrMenu() {
    return (Pointer.InMenu ? m_Linked_Menu : m_Linked_Content);
}

template <uint16_t W, uint16_t H>
std::vector<std::string> &Menu<W,H>::OthrMenu() {
    return (!Pointer.InMenu ? m_Linked_Menu : m_Linked_Content);
}