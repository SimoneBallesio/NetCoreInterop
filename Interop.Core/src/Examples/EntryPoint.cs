namespace Interop.Core.Examples;

public static partial class EntryPoint
{
	[UnmanagedCallersOnly]
	public static void PrintObjProperties(IntPtr obj)
	{
		CustomObject? decoded = Marshal.PtrToStructure<CustomObject>(obj);
		Console.WriteLine($"[C#]{nameof(PrintObjProperties)}: TextProp=\"{decoded?.TextProp}\"; DoubleProp={decoded?.DoubleProp}");
	}
};