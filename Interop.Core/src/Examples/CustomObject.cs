namespace Interop.Core.Examples;

/// <summary>
/// Sample class to test C++/C# interoperability
/// </summary>
[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
public struct CustomObject
{
	public CustomObject() {}

	[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
	public string TextProp = string.Empty;

	[MarshalAs(UnmanagedType.R8)]
	public double DoubleProp = 0.0;
};