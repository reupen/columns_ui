#pragma once

namespace cui::fb2k_utils {

template <class Service = service_base>
class LeakyServicePtr {
public:
    LeakyServicePtr() {}
    explicit LeakyServicePtr(service_ptr_t<Service> ptr) : m_ptr(std::move(ptr)) {}

    ~LeakyServicePtr()
    {
        assert(m_ptr.is_empty());
        (void)m_ptr.detach();
    }

    LeakyServicePtr(const LeakyServicePtr&) = delete;
    LeakyServicePtr& operator=(const LeakyServicePtr&) = delete;
    LeakyServicePtr(LeakyServicePtr&& other) = delete;
    LeakyServicePtr& operator=(LeakyServicePtr&& other) = delete;

    LeakyServicePtr& operator=(service_ptr_t<Service> ptr)
    {
        m_ptr = std::move(ptr);
        return *this;
    }

    operator const service_ptr_t<Service>&() const { return m_ptr; }
    operator bool() const { return m_ptr.valid(); }

    const service_ptr_t<Service>& get() { return m_ptr; }
    void reset() { m_ptr.reset(); }

private:
    service_ptr_t<Service> m_ptr;
};

} // namespace cui::fb2k_utils
