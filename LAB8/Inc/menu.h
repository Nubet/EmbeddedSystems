#ifndef MENU_H
#define MENU_H

void Menu_Init(void);
void Menu_PrintPromptForCurrentContext(void);
void Menu_ExecuteCommandInCurrentContext(char *line);

#endif // MENU_H
