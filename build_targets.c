/*
File:   build_targets.c
Author: Taylor Robbins
Date:   06\22\2026
Description: 
	** None
*/

#define MAX_DEPENDENCIES  4
typedef struct SampleDefinition SampleDefinition;
struct SampleDefinition
{
	const char* name;
	bool isCpp;
	bool hasShader;
	bool isComputeShader;
	bool useSokolDll;
	const char* dependencies[MAX_DEPENDENCIES];
};

u64 GetSampleDefinitions(SampleDefinition* defsOutBuffer)
{
	SampleDefinition definitions[] = {
		{ .name = "arraytex-sapp",              .hasShader=true },
		{ .name = "basisu-sapp",                .hasShader=false, .dependencies={ "basisu" } },
		{ .name = "blend-op-sapp",              .hasShader=true },
		{ .name = "blend-playground-sapp",      .hasShader=true, .dependencies={ "cimgui", "fileutil", "qoi" } },
		{ .name = "blend-sapp",                 .hasShader=true },
		{ .name = "bufferoffsets-sapp",         .hasShader=true },
		{ .name = "cgltf-sapp",                 .hasShader=true, .dependencies={ "basisu", "fileutil" } },
		{ .name = "cimgui-sapp",                .hasShader=false, .dependencies={ "cimgui" } },
		{ .name = "clear-sapp",                 .hasShader=false },
		{ .name = "computeboids-sapp",          .hasShader=true, .isComputeShader=true, .dependencies={ "cimgui" } },
		{ .name = "cube-sapp",                  .hasShader=true },
		{ .name = "cubemap-jpeg-sapp",          .hasShader=true, .dependencies={ "stb", "fileutil" } },
		{ .name = "cubemaprt-sapp",             .hasShader=true },
		{ .name = "cursor-sapp",                .hasShader=false, .isCpp=true, .dependencies={ "imgui" } },
		{ .name = "customresolve-sapp",         .hasShader=true, .dependencies={ "cimgui" } },
		{ .name = "debugtext-context-sapp",     .hasShader=true },
		{ .name = "debugtext-layers-sapp",      .hasShader=false },
		{ .name = "debugtext-printf-sapp",      .hasShader=false },
		{ .name = "debugtext-sapp",             .hasShader=false },
		{ .name = "debugtext-userfont-sapp",    .hasShader=false },
		{ .name = "drawcallperf-sapp",          .hasShader=true, .dependencies={ "cimgui" } },
		{ .name = "drawex-sapp",                .hasShader=true },
		{ .name = "droptest-sapp",              .hasShader=false, .dependencies={ "cimgui" } },
		{ .name = "dyntex-sapp",                .hasShader=true },
		{ .name = "dyntex3d-sapp",              .hasShader=true, .dependencies={ "cimgui" } },
		{ .name = "events-sapp",                .hasShader=false, .isCpp=true, .dependencies={ "imgui" } },
		{ .name = "fontstash-layers-sapp",      .hasShader=true, .dependencies={ "fileutil" } },
		{ .name = "fontstash-sapp",             .hasShader=false, .dependencies={ "fileutil" } },
		{ .name = "framebuffer-sapp",           .hasShader=false },
		{ .name = "icon-sapp",                  .hasShader=false },
		{ .name = "ilbm-sapp",                  .hasShader=false, .dependencies={ "cimgui", "ilbm", "fileutil" } },
		{ .name = "imageblur-sapp",             .hasShader=true, .isComputeShader=true, .dependencies={ "cimgui", "stb", "fileutil" } },
		{ .name = "imgui-dock-sapp",            .hasShader=false, .isCpp=true, .dependencies={ "imgui-docking" } },
		{ .name = "imgui-highdpi-sapp",         .hasShader=false, .isCpp=true, .dependencies={ "imgui" } },
		{ .name = "imgui-images-sapp",          .hasShader=false, .dependencies={ "cimgui" } },
		{ .name = "imgui-perf-sapp",            .hasShader=false, .dependencies={ "cimgui" } },
		{ .name = "imgui-sapp",                 .hasShader=false, .isCpp=true, .dependencies={ "imgui" } },
		{ .name = "imgui-usercallback-sapp",    .hasShader=true, .dependencies={ "cimgui" } },
		{ .name = "instancing-compute-sapp",    .hasShader=true, .isComputeShader=true },
		{ .name = "instancing-pull-sapp",       .hasShader=true },
		{ .name = "instancing-sapp",            .hasShader=true },
		{ .name = "layerrender-sapp",           .hasShader=true },
		{ .name = "letterbox-sapp",             .hasShader=false, .dependencies={ "cimgui" } },
		{ .name = "loadpng-sapp",               .hasShader=true, .dependencies={ "stb", "fileutil" } },
		{ .name = "mipmap-sapp",                .hasShader=true },
		{ .name = "miprender-sapp",             .hasShader=true },
		// { .name = "modplay-sapp",               .hasShader=false }, //TODO: Depends on https://github.com/Konstanty/libmodplug (see https://github.com/floooh/fibs-libs/blob/main/libmodplug.ts)
		{ .name = "mrt-pixelformats-sapp",      .hasShader=true },
		{ .name = "mrt-sapp",                   .hasShader=true },
		{ .name = "noentry-dll-sapp",           .hasShader=true, .useSokolDll=true },
		// { .name = "noentry-sapp",               .hasShader=true }, //TODO: Depends on noentry version of sokol_app.h
		{ .name = "noninterleaved-sapp",        .hasShader=true },
		{ .name = "nuklear-images-sapp",        .hasShader=false, .dependencies={ "nuklear" } },
		{ .name = "nuklear-sapp",               .hasShader=false, .dependencies={ "nuklear" } },
		{ .name = "offscreen-msaa-sapp",        .hasShader=true },
		{ .name = "offscreen-sapp",             .hasShader=true },
		// { .name = "ozz-anim-sapp",              .hasShader=false, .isCpp=true, .dependencies={ "imgui" } }, //TODO: Cannot open include file: 'ozz/animation/runtime/animation.h': No such file or directory
		// { .name = "ozz-skin-sapp",              .hasShader=true, .isCpp=true, .dependencies={ "imgui" } }, //TODO: Cannot open include file: 'ozz/animation/runtime/animation.h': No such file or directory
		// { .name = "ozz-storagebuffer-sapp",     .hasShader=true, .isCpp=true, .dependencies={ "imgui" } }, //TODO: Cannot open include file: 'ozz/animation/runtime/animation.h': No such file or directory
		{ .name = "pixelformats-sapp",          .hasShader=true, .dependencies={ "cimgui" } },
		{ .name = "plmpeg-sapp",                .hasShader=true, .dependencies={ "fileutil" } },
		{ .name = "primtypes-sapp",             .hasShader=true },
		{ .name = "quad-sapp",                  .hasShader=true },
		// { .name = "restart-sapp",               .hasShader=true }, //TODO: Depends on modplug.h
		{ .name = "saudio-sapp",                .hasShader=false },
		{ .name = "sbufoffset-sapp",            .hasShader=true, .isComputeShader=true },
		{ .name = "sbuftex-sapp",               .hasShader=true },
		{ .name = "sdf-sapp",                   .hasShader=true },
		{ .name = "sgl-context-sapp",           .hasShader=false },
		{ .name = "sgl-lines-sapp",             .hasShader=false },
		{ .name = "sgl-microui-sapp",           .hasShader=false, .dependencies={ "microui" } },
		{ .name = "sgl-points-sapp",            .hasShader=false },
		{ .name = "sgl-sapp",                   .hasShader=false },
		{ .name = "shadows-depthtex-sapp",      .hasShader=true },
		{ .name = "shadows-sapp",               .hasShader=true },
		{ .name = "shapes-sapp",                .hasShader=true },
		{ .name = "shapes-transform-sapp",      .hasShader=true },
		{ .name = "shared-bindings-sapp",       .hasShader=true },
		// { .name = "shdfeatures-sapp",           .hasShader=true, .dependencies={ "cimgui" } }, //TODO: Need a .glsl.none.h file?
		// { .name = "spine-contexts-sapp",        .hasShader=false }, //TODO: Depends on spine.h
		// { .name = "spine-inspector-sapp",       .hasShader=false }, //TODO: Depends on spine.h
		// { .name = "spine-layers-sapp",          .hasShader=false }, //TODO: Depends on spine.h
		// { .name = "spine-simple-sapp",          .hasShader=false }, //TODO: Depends on spine.h
		// { .name = "spine-skinsets-sapp",        .hasShader=false }, //TODO: Depends on spine.h
		// { .name = "spine-switch-skinsets-sapp", .hasShader=false }, //TODO: Depends on spine.h
		{ .name = "tex3d-sapp",                 .hasShader=true },
		{ .name = "texcube-sapp",               .hasShader=true },
		{ .name = "texview-sapp",               .hasShader=true, .dependencies={ "cimgui", "basisu", "fileutil" } },
		{ .name = "triangle-bufferless-sapp",   .hasShader=true },
		{ .name = "triangle-sapp",              .hasShader=true },
		{ .name = "uniformtypes-sapp",          .hasShader=true },
		{ .name = "uvwrap-sapp",                .hasShader=true },
		{ .name = "vertexindexbuffer-sapp",     .hasShader=true },
		{ .name = "vertexpull-sapp",            .hasShader=true },
		{ .name = "vertextexture-sapp",         .hasShader=true, .isComputeShader=true },
		{ .name = "write-storageimage-sapp",    .hasShader=true, .isComputeShader=true },
	};
	u64 numSamples = ArrayCount(definitions);
	if (defsOutBuffer != nullptr) { memcpy(defsOutBuffer, &definitions[0], sizeof(SampleDefinition) * numSamples); }
	return numSamples;
}
