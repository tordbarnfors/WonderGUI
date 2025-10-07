using System.Runtime.InteropServices;

namespace WG;

//____ C_Component ____________________________________________________________

[StructLayout(LayoutKind.Sequential)]
internal struct C_Component
{
    public IntPtr objekt;
    public IntPtr component;
}

//____ ErrorLevel _____________________________________________________________
public enum ErrorLevel
{
	Warning,
	SilentError,
	Error,
	Critical
}

//____ ErrorCode ______________________________________________________________
public enum ErrorCode
{
	OutOfRange,
	InvalidIterator,
	InvalidParam,
	FailedPrerequisite,
	OpenGL,
	Internal,
	SystemIntegrity,
	IllegalCall,
	ResourceExhausted,
	RenderFailure,
	Performance,
	Other
}