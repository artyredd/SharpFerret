#include "csharp.h"
#include "imaging.h"

// The way a window is displayed, 0 = windowed, 1 = BorderlessFullScreen, 2 = FullScreen
typedef int WindowMode;

const static WindowMode WINDOWED = 0;
const static WindowMode BORDERLESS_FULLSCREEN = 1;
const static WindowMode FULLSCREEN = 2;

/// <summary>
/// The game window
/// </summary>
typedef struct _window* Window;

struct _window {
	/// <summary>
	/// The width in pixels of this window
	/// </summary>
	size_t Width;
	/// <summary>
	/// The height in pixels of this window
	/// </summary>
	size_t Height;
	/// <summary>
	/// The underlying GLFWwindow handle for this window
	/// </summary>
	void* Handle;
	/// <summary>
	/// Initializes OpenGl(if it hasn't been already) and opens this window
	/// </summary>
	void(*Open)(Window);
	/// <summary>
	/// Disposes of this window
	/// </summary>
	void(*Dispose)(Window);
	/// <summary>
	/// Set's the window's icon
	/// </summary>
	void(*SetIcon)(Window, Image);
	/// <summary>
	/// Sets the mode of the window, 0 = WINDOWED, 1 = BORDERLESS_FULLSCREEN, 2 = FULLSCREEN
	/// </summary>
	void(*SetMode)(Window, WindowMode);
};

Window CreateWindow();