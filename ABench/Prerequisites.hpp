#pragma once

namespace ABench {

namespace Common {

class FBXFile;
class Library;
class Timer;
class Window;

} // namespace Common

namespace Math {

class Vector3;
class Vector4;
class Matrix;

} // namespace Math

namespace Renderer {

class Instance;
class Device;

class Backbuffer;
class Buffer;
class CommandBuffer;
class Debugger;
class DescriptorAllocator;
class Framebuffer;
class Pipeline;
class RingBuffer;
class Shader;
class Texture;
class Tools;
class VertexLayout;

using InstancePtr = std::shared_ptr<Instance>;
using DevicePtr = std::shared_ptr<Device>;
using TexturePtr = std::shared_ptr<Texture>;
using BufferPtr = std::shared_ptr<Buffer>;

class ParticleEngine;

} // namespace Renderer

namespace Scene {

class Camera;
class Component;
class Mesh;
class Object;
class Scene;

} // namespace Scene

} // namespace ABench
