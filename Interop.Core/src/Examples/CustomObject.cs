namespace Interop.Core.Examples;

[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
public class CustomObject
{
	[MarshalAs(UnmanagedType.LPStr)]
	public string TextProp = string.Empty;

	[MarshalAs(UnmanagedType.R8)]
	public double DoubleProp;
};