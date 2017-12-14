#pragma once


namespace ABench {
namespace Renderer {

/**
 * Wrapper for Vulkan objects, which will handle memory deallocation when going
 * out of scope.
 *
 * Memory cleaning function takes an object as an argument (preferably, it should be
 * a lambda with DevicePtr captured by value).
 *
 * By design, this template class should be possible to use like a regular Vk object.
 */
template <typename T>
class VkRAII
{
    using VkRAIICleaner = std::function<void(T)>;

    T mObject;
    VkRAIICleaner mCleaner;

public:
    // non-copyable, copying would require shared_ptr-like common block of reference counters
    VkRAII(const VkRAII& other) = delete;
    VkRAII& operator=(const VkRAII& other) = delete;

    VkRAII()
        : mObject(VK_NULL_HANDLE)
        , mCleaner()
    {
        LOGM("Constructing empty VkRAII object of type " << Common::TypeName<T>::Get());
    }

    VkRAII(T object, VkRAIICleaner cleaner)
        : mObject(object)
        , mCleaner(cleaner)
    {
        LOGM("Constructing VkRAII object of type " << Common::TypeName<T>::Get() << " holding object " << std::hex << mObject);
    }

    VkRAII(VkRAII&& other)
        : mObject(std::move(other.mObject))
        , mCleaner(std::move(other.mCleaner))
    {
        other.mObject = VK_NULL_HANDLE;
        LOGM("Moving VkRAII object of type " << Common::TypeName<T>::Get() << " holding object " << std::hex << mObject);
    }

    VkRAII& operator=(VkRAII&& other)
    {
        mObject = std::move(other.mObject);
        mCleaner = std::move(other.mCleaner);
        other.mObject = VK_NULL_HANDLE;

        LOGM("Move-assigning VkRAII object of type " << Common::TypeName<T>::Get() << " holding object " << std::hex << mObject);
        return *this;
    }

    ~VkRAII()
    {
        if (mObject != VK_NULL_HANDLE)
            mCleaner(mObject);

        LOGM("Destroyed VkRAII object of type " << Common::TypeName<T>::Get() << " and pointer " << std::hex << mObject);
    }

    operator T()
    {
        return mObject;
    }

    operator bool()
    {
        return (mObject != VK_NULL_HANDLE) && mCleaner;
    }
};

} // namespace Renderer
} // namespace ABench
