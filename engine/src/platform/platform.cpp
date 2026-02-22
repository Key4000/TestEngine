/*
* Реализация интерфейса платформенного 
* слоя 
*/
#include <platform/platform.hpp>
#include <core/logger.hpp>

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>

typedef struct internal_state{
  HINSTANCE h_instance; //экземпляр приложения 
  HWND hwnd;            //дескриптор окна 
}internal_state;

static f64 clock_frequency; //частота таймера 
static LARGE_INTEGER start_time;

/*
* Основной обработчик сообщений от 
* системы. 
* Он обрабатывает различные события, 
* такие как нажатие клавиш, движение
* LRESULT - специальный целочисленный тип 
* CALLBACK - макрос для winAPI
* (дескриптор окна)
* (идентификатор сообщения)
* (доп информация о сообщении)
* (доп информация о сообщении)
*/
LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param );
/*
* Создание , удаление окна
* внутренние функции , 
* лежат только здесь 
*/
b8 platform_create_window(window_data* w_data, internal_state* i_state);
void platform_destroy_window(window_data* w_data);
/*
* 
*/
//
b8 platform_init(platform_state* p_state,window_data* w_data)
{
  /*
  *
  */
  p_state->internal_state = malloc(sizeof(internal_state));
  internal_state* i_state = (internal_state *)plat_state->internal_state;
  i_state->h_instance = GetModuleHandleA(0);
  /*
  * создаем окно
  */
  if(!platform_create_window(w_data, i_state))
  {
    return false
  }
  /*
  * инициализация таймера
  */
  LARGE_INTEGER frequency;
  QueryPerformanceFrequency(&frequency);
  clock_frequency = 1.0 / (f64)frequency.QuadPart;
  QueryPerformanceCounter(&start_time);

  return true;
}
//
void platform_shutdown(platform_state* p_state)
{
  
};
//
b8 platform_pump_messages(platform_state* p_state)
{
  
};


//
b8 platform_create_window(window_data* w_data, internal_state* i_state)
{
  /*
  * Настройка и регистрация 
  * класса окна
  */
  HICON icon = LoadIcon(state->h_instance, IDI_APPLICATION);
  WNDCLASSA wc;
  //TODO: 0 на nullptr?
  memset(&wc, 0, sizeof(wc));
  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = win32_process_message;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = state->h_instance;
  wc.hIcon = icon;
  //TODO: NULL на nullptr?
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = NULL;       
  wc.lpszClassName = "te_window_class";
  if (!RegisterClassA(&wc)) 
  {
    MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
    return false;
  }
  /*      
  * создаем окно     
  */
  u32 client_x = w_data->x;
  u32 client_y = w_data->y;
  u32 client_width = w_data->width;
  u32 client_height = w_data->height;

  u32 window_x = client_x;
  u32 window_y = client_y;
  u32 window_width = client_width;
  u32 window_height = client_height;
  
  u32 window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
  u32 window_ex_style = WS_EX_APPWINDOW;
  window_style |= WS_MAXIMIZEBOX;
  window_style |= WS_MINIMIZEBOX;   
  window_style |= WS_THICKFRAME; 

  RECT border_rect = {0, 0, 0, 0};
  AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);
  window_x += border_rect.left;
  window_y += border_rect.top;
  window_width += border_rect.right - border_rect.left;
  window_height += border_rect.bottom - border_rect.top;
  
  HWND handle = CreateWindowExA(
    window_ex_style, "te_window_class", w_data->app_name,
    window_style, window_x, window_y,
    window_width, window_height,
    0, 0, i_state->h_instance, 0);
  if (handle == 0) 
  {
    MessageBoxA(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
    KFATAL("Window creation failed!");
    return false;
  } 
  else 
  {
    i_state->hwnd = handle;
  }
  /* Код отображение
  *  и активацией окна 
  */
  b32 should_activate = 1;  // TODO: если окно не должно принимать ввод должно быть false 
  //если окно активировано, то показать и активировать, если нет, то показать, но не активировать
  i32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;
  ShowWindow(state->hwnd, show_window_command_flags);
};
//
void platform_destroy_window(window_data* w_data)
{
  //TODO: 
};


