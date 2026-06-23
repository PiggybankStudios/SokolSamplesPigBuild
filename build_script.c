/*
File:   build_script.c
Author: Taylor Robbins
Date:   05\16\2026
Description: 
	** None
*/

#define PIG_BUILD_PRINT_SYS_CMDS 0
#include "pig_build.h"

#define DEBUG_BUILD                     1
#define COPY_TO_BIN                     1
#define ONLY_BUILD_NON_EXISTANT_SAMPLES 1

#if DEBUG_BUILD
#define IF_DEBUG(...)    __VA_ARGS__
#define IF_RELEASE(...)  //nothing
#else
#define IF_DEBUG(...)    //nothing
#define IF_RELEASE(...)  __VA_ARGS__
#endif

#define DOWNLOADED_FOLDER  "[ROOT]/downloaded"
#define SOKOL_FOLDER       DOWNLOADED_FOLDER "/sokol"
#define SOKOL_TOOLS_FOLDER DOWNLOADED_FOLDER "/sokol_tools"
#define DC_IMGUI_FOLDER    DOWNLOADED_FOLDER "/dcimgui"
#define STB_FOLDER         DOWNLOADED_FOLDER "/stb"
#define MICRO_UI_FOLDER    DOWNLOADED_FOLDER "/microui"
#define NUKLEAR_FOLDER     DOWNLOADED_FOLDER "/nuklear"

#if BUILDING_ON_WINDOWS
#define SHDC_BIN_PATH  SOKOL_TOOLS_FOLDER "/bin/win32/sokol-shdc.exe"
#elif BUILDING_ON_LINUX
#define SHDC_BIN_PATH  SOKOL_TOOLS_FOLDER "/bin/linux/sokol-shdc"
#elif BUILDING_ON_OSX_ARM
#define SHDC_BIN_PATH  SOKOL_TOOLS_FOLDER "/bin/osx_arm64/sokol-shdc"
#elif BUILDING_ON_OSX_INTEL
#define SHDC_BIN_PATH  SOKOL_TOOLS_FOLDER "/bin/osx/sokol-shdc"
#else
#error build_script.c SHDC_BIN_PATH needs to be updated to support the current platform!
#endif

#include "build_targets.c"

void FillCommonArguments();
void CompileSokolObjAndDll(Str objPath, Str dllPath, Str libPath);
void CompileImguiWithAndWithoutDocking(Str regularDllPath, Str regularLibPath, Str dockingDllPath, Str dockingLibPath);
bool CrossCompileShaderWithShdc(Str shaderGlslPath, Str shaderHeaderPath, Str shaderSrcPath, Str targetLanguages);
void CompileSample(SampleDefinition* def, Str exampleName, Str exampleFileName, Str exampleSrcPath, Str exampleBinName, Str shaderSrcPath, Str sokolLibFile, Str sokolObjFile);

void DownloadSokolIfNeeded();
void DownloadDCImguiIfNeeded();
void DownloadMicroUiIfNeeded();
void DownloadStbIfNeeded();
void DownloadNuklearIfNeeded();

static CliArgs commonArgs = EMPTY;
#if BUILDING_ON_WINDOWS
static bool isMsvcInitialized = false;
#endif

// +--------------------------------------------------------------+
// |                             Main                             |
// +--------------------------------------------------------------+
int main(int argc, char* argv[])
{
	RecompileIfNeeded(MakeStrArrayVa("../build_script.c", "../build_targets.c"));
	Str pigBuildFolder = StrLit(PIG_BUILD_ROOT);
	IF_WINDOWS(isMsvcInitialized = WasMsvcDevBatchRun());
	
	Str downloadedFolderResolvedRoot = ResolveRootTo(StrLit(DOWNLOADED_FOLDER), StrLit(".."));
	if (!DoesFolderExist(downloadedFolderResolvedRoot)) { mkdir(downloadedFolderResolvedRoot.chars, FOLDER_PERMISSIONS); }
	DownloadSokolIfNeeded();
	DownloadDCImguiIfNeeded();
	DownloadStbIfNeeded();
	DownloadMicroUiIfNeeded();
	DownloadNuklearIfNeeded();
	
	#if COPY_TO_BIN
	if (!DoesFolderExist(StrLit("../bin"))) { mkdir("../bin", FOLDER_PERMISSIONS); }
	#endif
	
	FillCommonArguments(&commonArgs);
	
	// +==============================+
	// | Compile sokol.obj/sokol.dll  |
	// +==============================+
	Str sokolObjFile = StrLit("sokol" OBJ_EXT);
	Str sokolDllFile = StrLit("sokol" DLL_EXT);
	Str sokolLibFile = StrLit("sokol" LIB_EXT);
	CompileSokolObjAndDll(sokolObjFile, sokolDllFile, sokolLibFile);
	#if COPY_TO_BIN
	CopyFileToFolder(sokolDllFile, StrLit("../bin"), true);
	#endif
	
	// +======================================+
	// | Compile imgui.dll/imgui_docking.dll  |
	// +======================================+
	Str imguiDllPath = StrLit("imgui" DLL_EXT);
	Str imguiLibPath = StrLit("imgui" LIB_EXT);
	Str dockingImguiDllPath = StrLit("imgui_docking" DLL_EXT);
	Str dockingImguiLibPath = StrLit("imgui_docking" LIB_EXT);
	CompileImguiWithAndWithoutDocking(imguiDllPath, imguiLibPath, dockingImguiDllPath, dockingImguiLibPath);
	#if COPY_TO_BIN
	CopyFileToFolder(imguiDllPath, StrLit("../bin"), true);
	CopyFileToFolder(dockingImguiDllPath, StrLit("../bin"), true);
	#endif
	
	// +--------------------------------------------------------------+
	// |                       Compile Samples                        |
	// +--------------------------------------------------------------+
	SampleDefinition* samples = malloc(sizeof(SampleDefinition) * GetSampleDefinitions(nullptr)); NotNull(samples);
	u64 numSamples = GetSampleDefinitions(samples);
	for (u64 sIndex = 0; sIndex < numSamples; sIndex++)
	{
		SampleDefinition def = samples[sIndex];
		Str exampleName = MakeStrNt(def.name);
		Str exampleFileName = JoinStrings2(exampleName, MakeStrNt(def.isCpp ? ".cc" : ".c"));
		Str exampleSrcPath = JoinStrings2(StrLit("[ROOT]/sapp/"), exampleFileName);
		Str exampleBinName = JoinStrings2(exampleName, StrLit(EXE_EXT));
		StrReplaceChars(exampleBinName, '-', '_');
		
		// +==============================+
		// |     Cross-Compile Shader     |
		// +==============================+
		Str shaderSrcPath = JoinStrings2(exampleName, def.isCpp ? StrLit(".glsl.cc") : StrLit(".glsl.c"));
		if (def.hasShader)
		{
			Str shaderGlslFileName = JoinStrings2(exampleName, StrLit(".glsl"));
			Str shaderGlslPath = JoinPaths(StrLit("[ROOT]/sapp/"), shaderGlslFileName);
			Str shaderHeaderPath = JoinStrings2(exampleName, StrLit(".glsl.h"));
			Str targetLanguages = MakeStrNt(def.isComputeShader ? "hlsl5:glsl430:metal_macos" : "hlsl5:glsl430:glsl300es:metal_macos");
			AssertFileExist(ResolveRootTo(shaderGlslPath, StrLit("..")), false);
			if (!CrossCompileShaderWithShdc(shaderGlslPath, shaderHeaderPath, shaderSrcPath, targetLanguages))
			{
				AssertFmt(false, "Failed to cross-compile \"%.*s\"!", StrPrint(shaderGlslPath));
			}
		}
		
		CompileSample(&samples[sIndex], exampleName, exampleFileName, exampleSrcPath, exampleBinName, shaderSrcPath, sokolLibFile, sokolObjFile);
		#if COPY_TO_BIN
		CopyFileToFolder(exampleBinName, StrLit("../bin"), true);
		#endif
	}
}

// +--------------------------------------------------------------+
// |                       Common Arguments                       |
// +--------------------------------------------------------------+
void FillCommonArguments()
{
	// +==============================+
	// |     MSVC Compiler Flags      |
	// +==============================+
	AddTaggedArg(&commonArgs, T_MSVC_CL, CL_FULL_FILE_PATHS);
	AddTaggedArg(&commonArgs, T_MSVC_CL, CL_NO_LOGO);
	AddTaggedArgNt(&commonArgs, T_MSVC_CL T_LANG_C, CL_LANG_VERSION, "clatest"); //Use latest C language spec features
	AddTaggedArgNt(&commonArgs, T_MSVC_CL T_LANG_C, CL_EXPERIMENTAL, "c11atomics"); //Enables _Atomic types
	AddTaggedArgNt(&commonArgs, T_MSVC_CL T_LANG_CPP, CL_LANG_VERSION, "c++20");
	AddTaggedArgNt(&commonArgs, T_MSVC_CL, CL_INCLUDE_DIR, ".");
	AddTaggedArgNt(&commonArgs, T_MSVC_CL, CL_INCLUDE_DIR, "[ROOT]/sapp");
	AddTaggedArgNt(&commonArgs, T_MSVC_CL, CL_INCLUDE_DIR, "[ROOT]/libs");
	AddTaggedArgNt(&commonArgs, T_MSVC_CL, CL_INCLUDE_DIR, SOKOL_FOLDER);
	AddTaggedArgNt(&commonArgs, T_MSVC_CL, CL_INCLUDE_DIR, SOKOL_FOLDER "/util");
	IF_DEBUG(AddTaggedArg(&commonArgs, T_MSVC_CL, CL_DEBUG_INFO));
	AddTaggedArgNt(&commonArgs, T_MSVC_CL, CL_OPTIMIZATION_LEVEL, DEBUG_BUILD ? "d" : "2");
	
	// +==============================+
	// |     Clang Compiler Flags     |
	// +==============================+
	AddTaggedArg(&commonArgs, T_CLANG, CLANG_FULL_FILE_PATHS);
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_DEFINE, DEBUG_BUILD ? "DEBUG_BUILD=1" : "DEBUG_BUILD=0");
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_OPTIMIZATION_LEVEL, DEBUG_BUILD ? "0" : "2");
	IF_DEBUG(AddTaggedArg(&commonArgs, T_CLANG, CLANG_DEBUG_INFO_DEFAULT));
	AddTaggedArgNt(&commonArgs, T_CLANG T_LANG_C, CLANG_LANG_VERSION, "gnu2x");
	AddTaggedArgNt(&commonArgs, T_CLANG T_LANG_CPP, CLANG_LANG_VERSION, "c++20");
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_INCLUDE_DIR, ".");
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_INCLUDE_DIR, "[ROOT]/sapp");
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_INCLUDE_DIR, "[ROOT]/libs");
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_INCLUDE_DIR, SOKOL_FOLDER);
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_INCLUDE_DIR, SOKOL_FOLDER "/util");
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_OUTPUT_FILE, "sokol_triangle");
	
	// +==============================+
	// |  Dependency Specific Flags   |
	// +==============================+
	AddTaggedArgNt(&commonArgs, "imgui",         CLI_QUOTED_ARG, "imgui" LIB_EXT);
	AddTaggedArgNt(&commonArgs, "cimgui",        CLI_QUOTED_ARG, "imgui" LIB_EXT);
	AddTaggedArgNt(&commonArgs, "imgui-docking", CLI_QUOTED_ARG, "imgui_docking" LIB_EXT);
	AddTaggedArgNt(&commonArgs, "fileutil",      CLI_QUOTED_ARG, "[ROOT]/libs/util/fileutil.c");
	AddTaggedArgNt(&commonArgs, "ilbm",          CLI_QUOTED_ARG, "[ROOT]/libs/ilbm/ilbm.c");
	AddTaggedArgNt(&commonArgs, "basisu",        CLI_QUOTED_ARG, "[ROOT]/libs/basisu/sokol_basisu.cpp");
	AddTaggedArgNt(&commonArgs, "microui",       CLI_QUOTED_ARG, MICRO_UI_FOLDER "/src/microui.c");
	AddTaggedArgNt(&commonArgs, "nuklear",       CLI_QUOTED_ARG, "[ROOT]/libs/nuklear/nuklear.c");
	
	AddTaggedArgNt(&commonArgs, T_MSVC_CL "|stb",           CL_INCLUDE_DIR,    STB_FOLDER);
	AddTaggedArgNt(&commonArgs, T_CLANG   "|stb",           CLANG_INCLUDE_DIR, STB_FOLDER);
	AddTaggedArgNt(&commonArgs, T_MSVC_CL "|imgui",         CL_INCLUDE_DIR,    DC_IMGUI_FOLDER "/src");
	AddTaggedArgNt(&commonArgs, T_CLANG   "|imgui",         CLANG_INCLUDE_DIR, DC_IMGUI_FOLDER "/src");
	AddTaggedArgNt(&commonArgs, T_MSVC_CL "|imgui-docking", CL_INCLUDE_DIR,    DC_IMGUI_FOLDER "/src-docking");
	AddTaggedArgNt(&commonArgs, T_CLANG   "|imgui-docking", CLANG_INCLUDE_DIR, DC_IMGUI_FOLDER "/src-docking");
	AddTaggedArgNt(&commonArgs, T_MSVC_CL "|cimgui",        CL_INCLUDE_DIR,    DC_IMGUI_FOLDER "/src");
	AddTaggedArgNt(&commonArgs, T_CLANG   "|cimgui",        CLANG_INCLUDE_DIR, DC_IMGUI_FOLDER "/src");
	AddTaggedArgNt(&commonArgs, T_MSVC_CL "|microui",       CL_INCLUDE_DIR,    MICRO_UI_FOLDER "/src");
	AddTaggedArgNt(&commonArgs, T_CLANG   "|microui",       CLANG_INCLUDE_DIR, MICRO_UI_FOLDER "/src");
	AddTaggedArgNt(&commonArgs, T_MSVC_CL "|microui",       CL_INCLUDE_DIR,    MICRO_UI_FOLDER "/demo");
	AddTaggedArgNt(&commonArgs, T_CLANG   "|microui",       CLANG_INCLUDE_DIR, MICRO_UI_FOLDER "/demo");
	AddTaggedArgNt(&commonArgs, T_MSVC_CL "|nuklear",       CL_INCLUDE_DIR,    NUKLEAR_FOLDER);
	AddTaggedArgNt(&commonArgs, T_CLANG   "|nuklear",       CLANG_INCLUDE_DIR, NUKLEAR_FOLDER);
	
	AddTaggedArgNt(&commonArgs, T_MSVC_CL "|qoi",     CL_DEFINE, "QOI_IMPLEMENTATION");
	AddTaggedArgNt(&commonArgs, T_MSVC_CL "|nuklear", CL_DISABLE_WARNING, "5287"); //operands are different enum types 'nk_edit_types' and 'nk_edit_flags'; use an explicit cast to silence this warning
	
	// +==============================+
	// |     Linux Compiler Flags     |
	// +==============================+
	AddTaggedArgNt(&commonArgs, T_CLANG T_LINUX, CLANG_SYSTEM_LIBRARY, "m");
	AddTaggedArgNt(&commonArgs, T_CLANG T_LINUX, CLANG_SYSTEM_LIBRARY, "GL");
	AddTaggedArgNt(&commonArgs, T_CLANG T_LINUX, CLANG_SYSTEM_LIBRARY, "X11");
	AddTaggedArgNt(&commonArgs, T_CLANG T_LINUX, CLANG_SYSTEM_LIBRARY, "Xi");
	AddTaggedArgNt(&commonArgs, T_CLANG T_LINUX, CLANG_SYSTEM_LIBRARY, "Xcursor");
	
	// +==============================+
	// |      OSX Compiler Flags      |
	// +==============================+
	AddTaggedArgNt(&commonArgs, T_CLANG T_OSX, CLANG_DEFINE, "TARGET_IS_OSX=1");
	AddTaggedArgNt(&commonArgs, T_CLANG T_OSX, CLANG_FRAMEWORK, "Cocoa");
	AddTaggedArgNt(&commonArgs, T_CLANG T_OSX, CLANG_FRAMEWORK, "QuartzCore");
	AddTaggedArgNt(&commonArgs, T_CLANG T_OSX, CLANG_FRAMEWORK, "CoreFoundation");
	AddTaggedArgNt(&commonArgs, T_CLANG T_OSX, CLANG_FRAMEWORK, "Metal");
	AddTaggedArgNt(&commonArgs, T_CLANG T_OSX, CLANG_FRAMEWORK, "MetalKit");
	
	AddTaggedArgNt(&commonArgs, T_MSVC_CL T_WINDOWS, CL_DEFINE,    "SOKOL_D3D11"); //SOKOL_D3D11, SOKOL_GLCORE, SOKOL_WGPU, SOKOL_VULKAN
	AddTaggedArgNt(&commonArgs, T_CLANG   T_OSX,     CLANG_DEFINE, "SOKOL_METAL"); //SOKOL_METAL, SOKOL_GLCORE
	AddTaggedArgNt(&commonArgs, T_CLANG   T_LINUX,   CLANG_DEFINE, "SOKOL_GLCORE"); //SOKOL_GLCORE, SOKOL_GLES3, SOKOL_WGPU, SOKOL_VULKAN
	
	// +==============================+
	// |         Linker Flags         |
	// +==============================+
	AddTaggedArg(&commonArgs, T_MSVC_CL, CL_LINK);
	AddTaggedArg(&commonArgs, T_MSVC_CL_OR_LINK, LINK_DISABLE_INCREMENTAL);
	AddTaggedArg(&commonArgs, T_MSVC_LINK, LINK_NO_LOGO);
}

// +--------------------------------------------------------------+
// |                     Compile Dependencies                     |
// +--------------------------------------------------------------+
void CompileSokolObjAndDll(Str objPath, Str dllPath, Str libPath)
{
	if (!DoesFileExist(objPath))
	{
		CliArgs args = EMPTY;
		AddTaggedArgNt(&args, T_CLANG T_OSX, "-x [VAL]", "objective-c");
		AddArgNt(&args, CLI_QUOTED_ARG, "[ROOT]/libs/sokol/sokol.c");
		AddTaggedArg(&args, T_MSVC_CL, CL_COMPILE);
		AddTaggedArg(&args, T_CLANG, CLANG_COMPILE);
		AddTaggedArgStr(&args, T_MSVC_CL, CL_BINARY_FILE, objPath);
		AddTaggedArgStr(&args, T_CLANG, CLANG_OUTPUT_FILE, objPath);
		AddTaggedArgNt(&args, T_MSVC_CL, CL_PDB_FILE, "sokol.pdb");
		AddArgList(&args, &commonArgs);
		
		#if BUILDING_ON_WINDOWS
		{
			PrintLine("[Building %.*s for WINDOWS...]", StrPrint(objPath));
			InitializeMsvcIf(StrLit(PIG_BUILD_ROOT), &isMsvcInitialized);
			RunCliProgramAndExitOnFailureTagsLit(StrLit(EXE_MSVC_CL), T_MSVC_CL T_WINDOWS T_LANG_C, &args, FormatStr("Failed to compile sokol.c into %.*s", StrPrint(objPath)));
			AssertFileExist(objPath, true);
			PrintLine("[Successfully built %.*s for WINDOWS!]", StrPrint(objPath));
		}
		#elif BUILDING_ON_OSX
		{
			PrintLine("[Building %.*s for OSX...]", StrPrint(objPath));
			RunCliProgramAndExitOnFailureTagsLit(StrLit(EXE_CLANG), T_CLANG T_OSX T_LANG_C, &args, FormatStr("Failed to compile sokol.c into %.*s", StrPrint(objPath)));
			AssertFileExist(objPath, true);
			PrintLine("[Successfully built %.*s for OSX!]", StrPrint(objPath));
		}
		#else
		#error The current platform is not supported yet!
		#endif
	}
	
	if (!DoesFileExist(dllPath))
	{
		CliArgs args = EMPTY;
		AddTaggedArgNt(&args, T_CLANG T_OSX, "-x [VAL]", "objective-c");
		AddArgNt(&args, CLI_QUOTED_ARG, "[ROOT]/libs/sokol/sokol-dll.c");
		AddTaggedArgStr(&args, T_MSVC_CL, CL_BINARY_FILE, dllPath);
		AddTaggedArgStr(&args, T_CLANG, CLANG_OUTPUT_FILE, dllPath);
		AddTaggedArgNt(&args, T_MSVC_CL, CL_PDB_FILE, "sokol_dll.pdb");
		AddArgList(&args, &commonArgs);
		AddTaggedArg(&args, T_MSVC_CL, LINK_BUILD_DLL);
		
		#if BUILDING_ON_WINDOWS
		{
			PrintLine("[Building %.*s for WINDOWS...]", StrPrint(dllPath));
			InitializeMsvcIf(StrLit(PIG_BUILD_ROOT), &isMsvcInitialized);
			RunCliProgramAndExitOnFailureTagsLit(StrLit(EXE_MSVC_CL), T_MSVC_CL T_WINDOWS T_LANG_C T_LIBRARY, &args, FormatStr("Failed to compile sokol-dll.c into %.*s", StrPrint(dllPath)));
			AssertFileExist(dllPath, true);
			PrintLine("[Successfully built %.*s for WINDOWS!]", StrPrint(dllPath));
		}
		#elif BUILDING_ON_OSX
		{
			PrintLine("[Building %.*s for OSX...]", StrPrint(dllPath));
			RunCliProgramAndExitOnFailureTagsLit(StrLit(EXE_CLANG), T_CLANG T_OSX T_LANG_C T_LIBRARY, &args, FormatStr("Failed to compile sokol-dll.c into %.*s", StrPrint(dllPath)));
			AssertFileExist(dllPath, true);
			PrintLine("[Successfully built %.*s for OSX!]", StrPrint(dllPath));
		}
		#else
		#error The current platform is not supported yet!
		#endif
	}
}

void CompileImguiWithAndWithoutDocking(Str regularDllPath, Str regularLibPath, Str dockingDllPath, Str dockingLibPath)
{
	Str dcImguiFolder = StrLit(DC_IMGUI_FOLDER);
	for (u64 dockingEnabled = 0; dockingEnabled < 2; dockingEnabled++)
	{
		Str libPath = MakeStrNt(dockingEnabled ? "imgui_docking" LIB_EXT : "imgui" LIB_EXT);
		Str dllPath = MakeStrNt(dockingEnabled ? "imgui_docking" DLL_EXT : "imgui" DLL_EXT);
		if (!DoesFileExist(dllPath) || !DoesFileExist(libPath))
		{
			PrintLine("[Building %.*s...]", StrPrint(dllPath));
			Str srcFolder = JoinPathsNt(dcImguiFolder, dockingEnabled ? "src-docking" : "src");
			StrArray sourceFiles = MakeStrArrayVaStr(
				JoinPathsLit(srcFolder, "cimgui.cpp"),
				JoinPathsLit(srcFolder, "cimgui_internal.cpp"),
				JoinPathsLit(srcFolder, "imgui_demo.cpp"),
				JoinPathsLit(srcFolder, "imgui_draw.cpp"),
				JoinPathsLit(srcFolder, "imgui_tables.cpp"),
				JoinPathsLit(srcFolder, "imgui_widgets.cpp"),
				JoinPathsLit(srcFolder, "imgui.cpp")
			);
			
			// +==============================+
			// |   Build Imgui Source Files   |
			// +==============================+
			StrArray objFiles = EMPTY;
			for (u64 sIndex = 0; sIndex < sourceFiles.length; sIndex++)
			{
				Str sourcePath = sourceFiles.strings[sIndex];
				Str objPath = ChangePathFolderAndExtension(sourcePath, Str_Empty, StrLit(OBJ_EXT), true);
				
				CliArgs args = EMPTY;
				AddArgStr(&args, CLI_QUOTED_ARG, sourcePath);
				AddTaggedArg(&args, T_MSVC_CL, CL_COMPILE);
				AddTaggedArg(&args, T_CLANG, CLANG_COMPILE);
				AddTaggedArgStr(&args, T_MSVC_CL, CL_BINARY_FILE, objPath);
				AddTaggedArgStr(&args, T_CLANG, CLANG_OUTPUT_FILE, objPath);
				AddTaggedArgNt(&args, T_MSVC_CL, CL_PDB_FILE, "imgui.pdb");
				IF_WINDOWS(AddTaggedArgNt(&args, T_MSVC_CL, CL_DEFINE, "CIMGUI_API=__declspec(dllexport)"));
				IF_WINDOWS(AddTaggedArgNt(&args, T_MSVC_CL, CL_DEFINE, "IMGUI_API=__declspec(dllexport)"));
				AddArgList(&args, &commonArgs);
				AddArgStr(&args, LINK_IMPORT_LIBRARY_FILE, libPath);
				
				StrArray tags = EMPTY;
				if (StrAnyCaseEquals(GetFileExtPart(sourcePath, false), StrLit(".c"))) { AddTag(&tags, T_LANG_C); }
				if (StrAnyCaseEquals(GetFileExtPart(sourcePath, false), StrLit(".cpp"))) { AddTag(&tags, T_LANG_CPP); }
				if (StrAnyCaseEquals(GetFileExtPart(sourcePath, false), StrLit(".cc"))) { AddTag(&tags, T_LANG_CPP); }
				
				#if BUILDING_ON_WINDOWS
				{
					AddTag(&tags, T_MSVC_CL);
					AddTag(&tags, T_MSVC_CL_OR_LINK);
					AddTag(&tags, T_WINDOWS);
					InitializeMsvcIf(StrLit(PIG_BUILD_ROOT), &isMsvcInitialized);
					RunCliProgramAndExitOnFailureTags(StrLit(EXE_MSVC_CL), tags, &args, FormatStr("Failed to compile %.*s into %.*s", StrPrint(sourcePath), StrPrint(objPath)));
					AssertFileExist(objPath, true);
				}
				#elif BUILDING_ON_OSX
				{
					AddTag(&tags, T_CLANG);
					AddTag(&tags, T_OSX);
					RunCliProgramAndExitOnFailureTags(StrLit(EXE_CLANG), tags, &args, FormatStr("Failed to compile %.*s into %.*s", StrPrint(sourcePath), StrPrint(objPath)));
					AssertFileExist(objPath, true);
				}
				#else
				#error The current platform is not supported yet!
				#endif
				
				AddStr(&objFiles, objPath);
			}
			
			// +==============================+
			// |        Link imgui.lib        |
			// +==============================+
			{
				CliArgs args = EMPTY;
				AddArg(&args, LINK_BUILD_DLL);
				for (u64 oIndex = 0; oIndex < objFiles.length; oIndex++)
				{
					AddArgStr(&args, CLI_QUOTED_ARG, objFiles.strings[oIndex]);
				}
				AddTaggedArgStr(&args, T_MSVC_LINK, LINK_OUTPUT_FILE, dllPath);
				AddTaggedArgStr(&args, T_MSVC_LINK, LINK_IMPORT_LIBRARY_FILE, libPath);
				AddArgList(&args, &commonArgs);
				
				StrArray tags = EMPTY;
				AddTag(&tags, T_LIBRARY);
				AddTag(&tags, "|imgui_lib");
				
				#if BUILDING_ON_WINDOWS
				{
					AddTag(&tags, T_MSVC_LINK);
					AddTag(&tags, T_MSVC_CL_OR_LINK);
					AddTag(&tags, T_WINDOWS);
					RunCliProgramAndExitOnFailureTags(StrLit(EXE_MSVC_LINK), tags, &args, FormatStr("Failed to link %.*s", StrPrint(dllPath)));
					AssertFileExist(dllPath, true);
					AssertFileExist(libPath, true);
				}
				#elif BUILDING_ON_OSX
				{
					AddTag(&tags, T_CLANG);
					AddTag(&tags, T_OSX);
					RunCliProgramAndExitOnFailureTags(StrLit(EXE_CLANG), tags, &args, FormatStr("Failed to link %.*s", StrPrint(dllPath)));
					AssertFileExist(dllPath, true);
					AssertFileExist(libPath, true);
				}
				#else
				#error The current platform is not supported yet!
				#endif
			}
			
			PrintLine("[Successfully built %.*s!]", StrPrint(dllPath));
		}
	}
}

bool CrossCompileShaderWithShdc(Str shaderGlslPath, Str shaderHeaderPath, Str shaderSrcPath, Str targetLanguages)
{
	Str shaderGlslFileName = GetFileNamePart(shaderGlslPath, true);
	if (!DoesFileExist(shaderHeaderPath))
	{
		PrintLine("Cross-compiling %.*s using sokol-shdc...", StrPrint(shaderGlslFileName));
		
		CliArgs shdcArgs = EMPTY;
		AddArgNt(&shdcArgs, SHDC_FORMAT, "sokol_impl");
		AddArgNt(&shdcArgs, SHDC_ERROR_FORMAT, "msvc");
		// AddArg(&shdcArgs, SHDC_REFLECTION);
		AddArgStr(&shdcArgs, SHDC_SHADER_LANGUAGES, targetLanguages);
		AddArgStr(&shdcArgs, SHDC_INPUT, shaderGlslPath);
		AddArgStr(&shdcArgs, SHDC_OUTPUT, shaderHeaderPath);
		
		Str shdcPathResolved = ResolveRootTo(StrLit(SHDC_BIN_PATH), StrLit(".."));
		FixPathSlashes(shdcPathResolved, PATH_SEP_CHAR);
		int exitCode = RunCliProgram(shdcPathResolved, &shdcArgs);
		FreeStr(&shdcPathResolved);
		if (exitCode != 0)
		{
			PrintLine("Failed to cross-compile %.*s using sokol-shdc! Exit code: %d", StrPrint(shaderGlslFileName), exitCode);
			return false;
		}
		else
		{
			AssertFileExist(shaderHeaderPath, true);
			PrintLine("Successfully cross-compiled %.*s!", StrPrint(shaderGlslFileName));
		}
	}
	
	if (!DoesFileExist(shaderSrcPath))
	{
		PrintLine("Creating %.*s", StrPrint(shaderSrcPath));
		Str cCode = FormatStr("\n"
			"#include \"sokol_gfx.h\"\n"
			"#include \"vecmath/vecmath.h\"\n"
			"#define SOKOL_SHDC_IMPL\n"
			"#include \"%.*s\"",
			StrPrint(shaderHeaderPath)
		);
		CreateAndWriteFile(shaderSrcPath, cCode, true);
	}
	
	return true;
}

// +--------------------------------------------------------------+
// |                        Compile Sample                        |
// +--------------------------------------------------------------+
void CompileSample(SampleDefinition* def, Str exampleName, Str exampleFileName, Str exampleSrcPath, Str exampleBinName, Str shaderSrcPath, Str sokolLibFile, Str sokolObjFile)
{
	#if ONLY_BUILD_NON_EXISTANT_SAMPLES
	if (DoesFileExist(exampleBinName)) { return; }
	#endif
	
	CliArgs args = EMPTY;
	AddArgStr(&args, CLI_QUOTED_ARG, exampleSrcPath);
	if (def->useSokolDll) { AddArgStr(&args, CLI_QUOTED_ARG, sokolLibFile); }
	else { AddArgStr(&args, CLI_QUOTED_ARG, sokolObjFile); }
	if (def->hasShader) { AddArgStr(&args, CLI_QUOTED_ARG, shaderSrcPath); }
	AddTaggedArgStr(&args, T_MSVC_CL, CL_BINARY_FILE, exampleBinName);
	AddTaggedArgStr(&args, T_CLANG, CLANG_OUTPUT_FILE, exampleBinName);
	IF_WINDOWS(AddTaggedArgStr(&args, T_MSVC_CL, CL_PDB_FILE, JoinStrings2(GetFileNamePart(exampleSrcPath, false), StrLit(".pdb"))));
	AddArgList(&args, &commonArgs);
	
	StrArray tags = EMPTY;
	if (def->isCpp) { AddTag(&tags, T_LANG_CPP); }
	else { AddTag(&tags, T_LANG_C); }
	for (u64 dIndex = 0; dIndex < ArrayCount(def->dependencies); dIndex++)
	{
		if (def->dependencies[dIndex] != nullptr) { AddStrNt(&tags, def->dependencies[dIndex]); }
	}
	
	#if BUILDING_ON_WINDOWS
	{
		PrintLine("[Building %.*s for WINDOWS...]", StrPrint(exampleBinName));
		InitializeMsvcIf(StrLit(PIG_BUILD_ROOT), &isMsvcInitialized);
		AddTag(&tags, T_MSVC_CL);
		AddTag(&tags, T_MSVC_CL_OR_LINK);
		AddTag(&tags, T_WINDOWS);
		RunCliProgramAndExitOnFailureTags(StrLit(EXE_MSVC_CL), tags, &args, FormatStr("Failed to compile %.*s into %.*s", StrPrint(exampleFileName), StrPrint(exampleBinName)));
		AssertFileExist(exampleBinName, true);
		PrintLine("[Successfully built %.*s for WINDOWS!]", StrPrint(exampleBinName));
	}
	#elif BUILDING_ON_OSX
	{
		PrintLine("[Building %.*s for OSX...]", StrPrint(exampleBinName));
		AddTag(&tags, T_CLANG);
		AddTag(&tags, T_OSX);
		RunCliProgramAndExitOnFailureTags(StrLit(EXE_CLANG), tags, &args, FormatStr("Failed to compile %.*s into %.*s", StrPrint(exampleFileName), StrPrint(exampleBinName)));
		AssertFileExist(exampleBinName, true);
		PrintLine("[Successfully built %.*s for OSX!]", StrPrint(exampleBinName));
	}
	#else
	#error The current platform is not supported yet!
	#endif
}

// +--------------------------------------------------------------+
// |                    Download Dependencies                     |
// +--------------------------------------------------------------+
void DownloadSokolIfNeeded()
{
	// https://github.com/floooh/sokol/commit/453c71214fbb55d782683d20ea7e6c07314e3e9b
	// Commit 453c712 from May 14th 2026 - "sokol_framebuffer.h: fix some comment typos"
	Str sokolUrl = StrLit("https://github.com/floooh/sokol/archive/453c71214fbb55d782683d20ea7e6c07314e3e9b.zip");
	Str sokolZipPath = StrLit("sokol_453c712.zip");
	Str sokolZipRootFolder = StrLit("sokol-453c71214fbb55d782683d20ea7e6c07314e3e9b");
	Str sokolFolderResolved = ResolveRootTo(StrLit(SOKOL_FOLDER), StrLit(".."));
	if (!DoesFileExist(sokolZipPath) || !DoesFolderExist(sokolFolderResolved))
	{
		PrintLine("Downloading Sokol from \"%.*s\"", StrPrint(sokolUrl));
		// if (DoesFolderExist(sokolFolderPath)) { MyRemoveDirectory(sokolFolderPath, true); } //TODO: Enable me once MyRemoveDirectory is implemented for OSX/Linux!
		DownloadAndExtractArchive(
			sokolUrl,
			sokolZipPath,
			1573414, 0x5707750981F0E26C,
			sokolFolderResolved,
			sokolZipRootFolder
		);
	}
	
	// https://github.com/floooh/sokol-tools-bin/commit/1a9a4e54090fec42c5d13169b638f09f25474953
	// Commit 1a9a4e5 from April 26th 2026 - "updated (88)"
	Str sokolToolsUrl = StrLit("https://github.com/floooh/sokol-tools-bin/archive/1a9a4e54090fec42c5d13169b638f09f25474953.zip");
	Str sokolToolsZipPath = StrLit("sokol_tools_1a9a4e5.zip");
	Str sokolToolsZipRootFolder = StrLit("sokol-tools-bin-1a9a4e54090fec42c5d13169b638f09f25474953");
	Str sokolToolsFolderResolved = ResolveRootTo(StrLit(SOKOL_TOOLS_FOLDER), StrLit(".."));
	if (!DoesFileExist(sokolToolsZipPath) || !DoesFolderExist(sokolToolsFolderResolved))
	{
		PrintLine("Downloading Sokol Tools from \"%.*s\"", StrPrint(sokolToolsUrl));
		// if (DoesFolderExist(sokolToolsFolderPath)) { MyRemoveDirectory(sokolToolsFolderPath, true); } //TODO: Enable me once MyRemoveDirectory is implemented for OSX/Linux!
		DownloadAndExtractArchive(
			sokolToolsUrl,
			sokolToolsZipPath,
			18032476, 0xDF94D1D90D33715F,
			sokolToolsFolderResolved,
			sokolToolsZipRootFolder
		);
		
		// Mark sokol-shdc as executable on Linux and OSX
		#if BUILDING_ON_OSX || BUILDING_ON_LINUX
		CliArgs chmodArgs = EMPTY;
		AddArg(&chmodArgs, "+x");
		AddArgNt(&chmodArgs, CLI_QUOTED_ARG, SHDC_BIN_PATH);
		RunCliProgramAndExitOnFailure(StrLit("chmod"), "", &chmodArgs, StrLit("Failed to make sokol-shdc executable!"));
		#endif
	}
}

void DownloadDCImguiIfNeeded()
{
	// https://github.com/floooh/dcimgui/commit/f276b9ce0d03004916441daa28504c686cd37ecc
	// Commit f276b9c from April 15th 2026 - "build.zig: remove zig 0.15 support"
	Str dcImguiUrl = StrLit("https://github.com/floooh/dcimgui/archive/f276b9ce0d03004916441daa28504c686cd37ecc.zip");
	Str dcImguiZipPath = StrLit("dcimgui_v1.92.7.zip");
	Str dcImguiZipRootFolder = StrLit("dcimgui-f276b9ce0d03004916441daa28504c686cd37ecc");
	Str dcImguiFolderResolved = ResolveRootTo(StrLit(DC_IMGUI_FOLDER), StrLit(".."));
	if (!DoesFileExist(dcImguiZipPath) || !DoesFolderExist(dcImguiFolderResolved))
	{
		PrintLine("Downloading dcimgui from \"%.*s\"", StrPrint(dcImguiUrl));
		// if (DoesFolderExist(dcImguiFolderPath)) { MyRemoveDirectory(dcImguiFolderPath, true); } //TODO: Enable me once MyRemoveDirectory is implemented for OSX/Linux!
		DownloadAndExtractArchive(
			dcImguiUrl,
			dcImguiZipPath,
			2434694, 0xCB162595C1A06724,
			dcImguiFolderResolved,
			dcImguiZipRootFolder
		);
	}
}

void DownloadStbIfNeeded()
{
	// https://github.com/nothings/stb/commit/31c1ad37456438565541f4919958214b6e762fb4
	// Commit 31c1ad3 from April 15th 2026 - "Update CONTRIBUTING.md with anti-LLM link"
	Str stbUrl = StrLit("https://github.com/nothings/stb/archive/31c1ad37456438565541f4919958214b6e762fb4.zip");
	Str stbZipPath = StrLit("stb_31c1ad3.zip");
	Str stbZipRootFolder = StrLit("stb-31c1ad37456438565541f4919958214b6e762fb4");
	Str stbFolderResolved = ResolveRootTo(StrLit(STB_FOLDER), StrLit(".."));
	if (!DoesFileExist(stbZipPath) || !DoesFolderExist(stbFolderResolved))
	{
		PrintLine("Downloading stb from \"%.*s\"", StrPrint(stbUrl));
		// if (DoesFolderExist(stbFolderPath)) { MyRemoveDirectory(stbFolderPath, true); } //TODO: Enable me once MyRemoveDirectory is implemented for OSX/Linux!
		DownloadAndExtractArchive(
			stbUrl,
			stbZipPath,
			1757307, 0x415890FA5CBAC37E,
			stbFolderResolved,
			stbZipRootFolder
		);
	}
}

void DownloadMicroUiIfNeeded()
{
	// https://github.com/rxi/microui/commit/0850aba860959c3e75fb3e97120ca92957f9d057
	// Commit 0850aba from August 13th 2024 - "Version 2.02"
	Str microUiUrl = StrLit("https://github.com/rxi/microui/archive/0850aba860959c3e75fb3e97120ca92957f9d057.zip");
	Str microUiZipPath = StrLit("microUi_2.02.zip");
	Str microUiZipRootFolder = StrLit("microui-0850aba860959c3e75fb3e97120ca92957f9d057");
	Str microUiFolderResolved = ResolveRootTo(StrLit(MICRO_UI_FOLDER), StrLit(".."));
	if (!DoesFileExist(microUiZipPath) || !DoesFolderExist(microUiFolderResolved))
	{
		PrintLine("Downloading microui from \"%.*s\"", StrPrint(microUiUrl));
		// if (DoesFolderExist(microUiFolderPath)) { MyRemoveDirectory(microUiFolderPath, true); } //TODO: Enable me once MyRemoveDirectory is implemented for OSX/Linux!
		DownloadAndExtractArchive(
			microUiUrl,
			microUiZipPath,
			33738, 0x0188468AEB8DDC90,
			microUiFolderResolved,
			microUiZipRootFolder
		);
	}
}

void DownloadNuklearIfNeeded()
{
	// https://github.com/Immediate-Mode-UI/Nuklear
	// v4.13.3 from May 4th 2026 - https://github.com/Immediate-Mode-UI/Nuklear/releases/tag/v4.13.3
	Str nuklearUrl = StrLit("https://github.com/Immediate-Mode-UI/Nuklear/archive/refs/tags/v4.13.3.zip");
	Str nuklearZipPath = StrLit("nuklear_v4.13.3.zip");
	Str nuklearZipRootFolder = StrLit("Nuklear-4.13.3");
	Str nuklearFolderResolved = ResolveRootTo(StrLit(NUKLEAR_FOLDER), StrLit(".."));
	if (!DoesFileExist(nuklearZipPath) || !DoesFolderExist(nuklearFolderResolved))
	{
		PrintLine("Downloading nuklear from \"%.*s\"", StrPrint(nuklearUrl));
		// if (DoesFolderExist(nuklearFolderPath)) { MyRemoveDirectory(nuklearFolderPath, true); } //TODO: Enable me once MyRemoveDirectory is implemented for OSX/Linux!
		DownloadAndExtractArchive(
			nuklearUrl,
			nuklearZipPath,
			2452230, 0x43B1710B6B246AF2,
			nuklearFolderResolved,
			nuklearZipRootFolder
		);
	}
}
