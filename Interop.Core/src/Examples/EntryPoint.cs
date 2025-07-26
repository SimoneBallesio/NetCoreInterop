namespace Interop.Core.Examples;

public static partial class EntryPoint
{
	/// <summary>
	/// Delegate function declaration for functions manipulating <c>CustomObject</c>s
	/// </summary>
	/// <param name="obj" cref="CustomObject">Pointer to a <c>CustomObject</c> object</param>
	/// <returns><c>true</c> on successful completion of the task, else <c>false</c></returns>
	private delegate bool ParseCustomObjectFn(IntPtr obj);

	/// <summary>
	/// Prints to console the properties of the <c>CustomObject</c> provided
	/// </summary>
	/// <param name="obj" cref="CustomObject">Pointer to a <c>CustomObject</c> object</param>
	/// <returns><c>true</c> on successful completion of the task, else <c>false</c></returns>
	private static bool PrintCustomObjProperties(IntPtr obj)
	{
		CustomObject decoded = Marshal.PtrToStructure<CustomObject>(obj);

		Console.WriteLine($"[C#] {nameof(PrintCustomObjProperties)}: TextProp=\"{decoded.TextProp}\"; DoubleProp={decoded.DoubleProp}");

		return true;
	}

	/// <summary>
	/// Edits the properties of the <c>CustomObject</c> provided and prints them to console
	/// </summary>
	/// <param name="obj" cref="CustomObject">Pointer to a <c>CustomObject</c> object</param>
	/// <returns><c>true</c> on successful completion of the task, else <c>false</c></returns>
	private static bool EditCustomObjProperties(IntPtr obj)
	{
		CustomObject decoded = Marshal.PtrToStructure<CustomObject>(obj);

		decoded.TextProp = "Dolor sit amet";
		decoded.DoubleProp = 3.145628;

		Console.WriteLine($"[C#] {nameof(EditCustomObjProperties)}: TextProp=\"{decoded.TextProp}\"; DoubleProp={decoded.DoubleProp}");

		return true;
	}

	/// <summary>
	/// Prints to console the properties of the unmanaged <c>CustomObject</c> provided
	/// </summary>
	/// <param name="obj" cref="CustomObject">Pointer to a <c>CustomObject</c> object</param>
	[UnmanagedCallersOnly]
	public static void PrintObjProperties(IntPtr obj)
	{
		CustomObject? decoded = Marshal.PtrToStructure<CustomObject>(obj);
		Console.WriteLine($"[C#] {nameof(PrintObjProperties)}: TextProp=\"{decoded?.TextProp}\"; DoubleProp={decoded?.DoubleProp}");
	}

	/// <summary>
	/// Passes a managed <c>CustomObject</c> to the host program
	/// </summary>
	[UnmanagedCallersOnly]
	public static void PassObjectToHost()
	{
		CustomObject objToPass = new() { TextProp = "Dolor Sit", DoubleProp = 324.7677 };

		IntPtr buffer = Marshal.AllocCoTaskMem(Marshal.SizeOf(objToPass));
		Marshal.StructureToPtr(objToPass, buffer, false);

		PrintHostedObjProperties(buffer);

		Marshal.FreeCoTaskMem(buffer);
	}

	/// <summary>
	/// Passes a managed <c>CustomObject</c> to the host program
	/// which, in turn, calls a managed delegate function on the object
	/// </summary>
	[UnmanagedCallersOnly]
	public static void DelegateRoundabout()
	{
		CustomObject objToPass = new() { TextProp = "Amet", DoubleProp = 1123.567 };

		IntPtr buffer = Marshal.AllocCoTaskMem(Marshal.SizeOf(objToPass));
		Marshal.StructureToPtr(objToPass, buffer, false);

		ProcessCustomObject(buffer, PrintCustomObjProperties);
		ProcessCustomObject(buffer, EditCustomObjProperties);

		Marshal.FreeCoTaskMem(buffer);
	}

	/// <summary>
	/// Reads a <c>CustomObject</c> from shared memory, treated as a <c>CustomObject[]</c>,
	/// at the index specified. As a test function, it does not perform out of bounds checks.
	/// </summary>
	/// <param name="index">The index from which to read the <c>CustomObject</c></param>
	[UnmanagedCallersOnly]
	public static void ReadObjectFromSharedMemory(int index)
	{
		using var mmf = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
			? MemoryMappedFile.OpenExisting("Controller")
			: MemoryMappedFile.CreateFromFile("/tmp/Controller", FileMode.Open);

		int customObjSize = Marshal.SizeOf<CustomObject>();

		using var view = mmf.CreateViewStream(index * customObjSize, customObjSize);

		var buffer = new byte[customObjSize];
		var handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);

		int byteRead = view.Read(buffer, 0, customObjSize);

		if (byteRead != customObjSize)
		{
			Console.WriteLine($"[C#] {nameof(ReadObjectFromSharedMemory)} failed, read {byteRead} bytes out of {customObjSize} expected");
			handle.Free();
			return;
		}

		var decoded = Marshal.PtrToStructure<CustomObject>(handle.AddrOfPinnedObject());
		handle.Free();

		Console.WriteLine($"[C#] {nameof(ReadObjectFromSharedMemory)}: TextProp=\"{decoded.TextProp}\"; DoubleProp={decoded.DoubleProp}");
	}

	/// <summary>
	/// Writes a <c>CustomObject</c> to shared memory, treated as a <c>CustomObject[]</c>,
	/// at the index specified. As a test function, it does not perform out of bounds checks.
	/// </summary>
	/// <param name="index">The index to which to write the <c>CustomObject</c></param>
	[UnmanagedCallersOnly]
	public static void WriteObjectToSharedMemory(int index)
	{
		using var mmf = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
			? MemoryMappedFile.OpenExisting("Controller")
			: MemoryMappedFile.CreateFromFile("/tmp/Controller", FileMode.Open);

		int customObjSize = Marshal.SizeOf<CustomObject>();

		using var view = mmf.CreateViewStream(index * customObjSize, customObjSize);

		var buffer = new byte[customObjSize];
		var handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);

		CustomObject objToWrite = new() { TextProp = "Quare Id Faciam", DoubleProp = 75.46943 };
		Marshal.StructureToPtr(objToWrite, handle.AddrOfPinnedObject(), true);
		handle.Free();

		view.Write(buffer, 0, customObjSize);
	}

	/// <summary>
	/// Receives a managed <c>CustomObject</c> from the hosted managed library
	/// and prints out its properties to standard output
	/// </summary>
	/// <param name="obj" cref="CustomObject">The object to process</param>
	[LibraryImport("InteropLib")]
	private static partial void PrintHostedObjProperties(IntPtr obj);

	/// <summary>
	/// Receives a managed <c>CustomObject</c> from the hosted managed library
	/// and calls the passed managed delegate function on the object
	/// </summary>
	/// <param name="obj" cref="CustomObject">The object to process</param>
	/// <param name="callback" cref="ParseCustomObjectFn">Delegate function called on the object passed</param>
	[LibraryImport("InteropLib")]
	private static partial void ProcessCustomObject(IntPtr obj, ParseCustomObjectFn callback);
};