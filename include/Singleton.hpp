#pragma once
#include <map>
#include <memory>
#include <mutex>

namespace IngameIME
{
template <typename T, typename Arg>
class Singleton
{
  protected:
    using RefHolderType = std::map<Arg, std::weak_ptr<T>>;

  protected:
    static std::mutex    RefHolderMutex;
    static RefHolderType WeakRefs;

  private:
    const Arg arg;

  public:
    static std::shared_ptr<T> getOrCreate(Arg arg)
    {
        std::lock_guard guard(RefHolderMutex);
        auto            iter = WeakRefs.find(arg);

        std::shared_ptr<T> result;
        if (iter == WeakRefs.end() || !(result = iter->second.lock()))
        {
            result        = std::make_shared<T>(arg);
            WeakRefs[arg] = result;
        }
        return result;
    }

  public:
    Singleton(const Singleton&) = delete;
    Singleton(Singleton&&)      = delete;

  protected:
    Singleton(const Arg arg)
        : arg(arg)
    {
    }

  public:
    ~Singleton()
    {
        WeakRefs.erase(arg);
    }
};
} // namespace IngameIME
