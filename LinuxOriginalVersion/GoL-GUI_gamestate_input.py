import tkinter as tk

class Button:
    deadColor = "black"
    activeDeadColor = "green"
    
    liveColor = "white"
    activeLiveColor = "red"
    
    def __init__(self, row, col, frame):
        self.row = row
        self.col = col
        self.frame = frame

        if GUI_Layout.button_state[self.row][self.col] == 0:
            self.b = tk.Button(self.frame, bg = Button.deadColor, activebackground = Button.activeDeadColor, relief = "flat")
        else:
            self.b = tk.Button(self.frame, bg = Button.liveColor, activebackground = Button.activeLiveColor, relief = "flat")
        self.b.bind("<Button-1>", self.react)
        self.b.grid(row = self.row, column = self.col)

    def react(self, frame):
        if self.b.cget("bg") == Button.deadColor:
            self.b.config(bg = Button.liveColor, activebackground = Button.activeLiveColor)
            GUI_Layout.button_state[self.row][self.col] = 1
        else:
            self.b.config(bg = Button.deadColor, activebackground = Button.activeDeadColor)
            GUI_Layout.button_state[self.row][self.col] = 0

class ControlButton:
    def __init__(self, frame):
        self.frame = frame

        self.b = tk.Button(self.frame, text = 'Import gamestate', bg = "cyan", activebackground = 'yellow')
        self.b.bind("<Button-1>", self.save_nonGUI_gamestate)
        self.b.pack(fill = 'x')

    def save_nonGUI_gamestate(self, frame):
        file = open('_gamestate_.txt', 'w', encoding = 'ASCII')

        out = ""
        for row in GUI_Layout.MAX_ROW:
            for col in GUI_Layout.MAX_COL:
                out += str(GUI_Layout.button_state[row][col])
            out += '\n'

        file.write(out)
        file.close()

        print("\n--GUI input Mode terminated successfully--\n")
        exit(0)
        
class GUI_Layout:
    #Load non-GUI gamestate to buttons states.
    file = open('_gamestate_.txt', 'r', encoding = 'ASCII')
    out = file.read()
    file.close()

    out_lst = out.split('\n')

    MAX_COL_num = int(out_lst[-1]) - 2
    MAX_COL = range(MAX_COL_num)
    MAX_ROW = range(int(out_lst[-2]) - 2)

    button_state = []
    col_lst = []
    for row in MAX_ROW:
        for col in MAX_COL:
            col_lst.append(int(out_lst[row][col]))
        button_state.append(col_lst[-MAX_COL_num:])
    #Load non-GUI gamestate to buttons states.
    
    def __init__(self, root):
        self.r = root
            
        self.f_buttons = tk.Frame(root)
        for row in GUI_Layout.MAX_ROW:
            for col in GUI_Layout.MAX_COL:
                button = Button(row, col, self.f_buttons)
        self.f_buttons.pack()
            
        self.f_control = tk.Frame(root)
        saveState = ControlButton(self.f_control)
        self.f_control.pack(fill = 'x')

def main():
    root = tk.Tk()
    root.title("GUI input Mode (Gamestate)")
    root.resizable(False, False)

    Gamestate = GUI_Layout(root)

    root.mainloop()

    print("\n* GUI input Mode terminated unsuccessfully! Gamestate wasn't imported! *\n")

if __name__ == "__main__":
    main()
