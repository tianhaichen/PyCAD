import wx
from wx import glcanvas
import sys
import math
import cad
import Mouse
import Key
from RefreshObserver import RefreshObserver
import copy

graphics_canvases = []
repaint_registered = False

def OnRepaint():
    global graphics_canvases
    for g in graphics_canvases:
        g.Refresh()

class GraphicsCanvas(glcanvas.GLCanvas):
    def __init__(self, parent):
        glcanvas.GLCanvas.__init__(self, parent,-1, attribList=[glcanvas.WX_GL_RGBA, glcanvas.WX_GL_DOUBLEBUFFER, glcanvas.WX_GL_DEPTH_SIZE, 24])
        self.context = glcanvas.GLContext(self)
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_MOUSE_EVENTS, self.OnMouse)
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)
        self.Bind(wx.EVT_MENU, self.OnMenu, None, 10000, 12000)
        self.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        self.Bind(wx.EVT_KEY_UP, self.OnKeyUp)
        self.viewport= cad.Viewport()
        self.Resize()
        self.paint_callbacks = []
        self.context_menu_enabled = True
        self.middle_down = False
        self.observer = RefreshObserver(self)
        cad.RegisterObserver(self.observer)
        global repaint_registered
        if not repaint_registered:
            cad.RegisterOnRepaint(OnRepaint)
            repaint_registered = True
        graphics_canvases.append(self)
        self.right_down_and_no_left_clicked = False

    def OnSize(self, event):
       self.Resize()
       event.Skip()

    def OnMenu(self, event):
      index = event.GetId() - 10000
      tool = self.tools[index]
      tool.Run()
      
    def OnKeyDown(self, event):
        k = event.GetKeyCode()
        if k == wx.WXK_ESCAPE and wx.GetApp().frame.IsFullScreen():
            wx.GetApp().ShowFullScreen(False)
        else:
            key_code = Key.KeyCodeFromWx(event)
            if not wx.GetApp().input_mode_object.OnKeyDown(key_code):
                wx.GetApp().OnKeyDown(event)
            
    def OnKeyUp(self, event):
        key_code = Key.KeyCodeFromWx(event)
        wx.GetApp().input_mode_object.OnKeyUp(key_code)
    
    def AppendToolsToMenu(self, menu, tools):
      for tool in tools:
         if tool.IsSeparator():
            menu.AppendSeparator()
         elif tool.IsAToolList():
            sub_menu = wx.Menu()
            self.AppendToolsToMenu(sub_menu, tool.GetChildTools())
            menu.AppendMenu(wx.ID_ANY, tool.GetTitle(), sub_menu)
         else:
            item = wx.MenuItem(menu, 10000 + self.next_tool_id, text = tool.GetTitle(), help = tool.GetToolTip())
            str = tool.BitmapPath()
            if len(str)>0:
                try:
                    image = wx.Image(res_folder + '/bitmaps/' + str + '.png')
                    image.Rescale(24, 24)
                    item.SetBitmap(wx.BitmapFromImage(image))
                except:
                    pass
            menu.AppendItem(item)
            self.next_tool_id = self.next_tool_id + 1
            self.tools.append(tool)

    def OnMouse(self, event):
        self.SetCurrent(self.context)
        e = Mouse.MouseEventFromWx(event)
        
        if event.controlDown:
            wx.GetApp().select_mode.OnMouse(e)
            return                    
        
        if event.RightDown():
            self.right_down_and_no_left_clicked = True
        if event.LeftIsDown(): 
            self.right_down_and_no_left_clicked = False
            
        if event.RightUp() and self.right_down_and_no_left_clicked:
            wx.GetApp().DoDropDownMenu(self, event.GetX(), event.GetY(), event.ControlDown())
            self.right_down_and_no_left_clicked = False
        else:
            wx.GetApp().input_mode_object.OnMouse(e)

        event.Skip()

    def OnEraseBackground(self, event):
      pass # Do nothing, to avoid flashing on MSW

    def Resize(self):
      s = self.GetClientSize()
      self.viewport.WidthAndHeightChanged(s.GetWidth(), s.GetHeight())
      self.Refresh()

    def OnPaint(self, event):
      dc = wx.PaintDC(self)
      self.SetCurrent(self.context)
      self.viewport.glCommands()
      for callback in self.paint_callbacks:
          callback()
# 
#     // draw the input mode text on the top
#     if (m_graphics_text_mode != GraphicsTextModeNone)
#     {
#         std::wstring screen_text1, screen_text2;
# 
#         if (input_mode_object && input_mode_object->GetTitle())
#         {
#             screen_text1.append(input_mode_object->GetTitle());
#             screen_text1.append(L"\n");
#         }
#         if (m_graphics_text_mode == GraphicsTextModeWithHelp && input_mode_object)
#         {
#             const wchar_t* help_str = input_mode_object->GetHelpText();
#             if (help_str)
#             {
#                 screen_text2.append(help_str);
#             }
#         }
#         render_screen_text(screen_text1.c_str(), screen_text2.c_str());
#     }
      
      self.SwapBuffers()
      self.viewport.render_on_front_done = False
      wx.GetApp().DrawFront()
      
      return
