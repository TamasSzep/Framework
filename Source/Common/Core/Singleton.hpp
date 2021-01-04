// Singleton.hpp

#ifndef _CORE_SINGLETON_HPP_
#define _CORE_SINGLETON_HPP_

namespace Core
{
	template <typename Type>
	class Singleton
	{
	protected:

		Singleton() {}

		Singleton(const Singleton&) = delete;
		Singleton& operator=(const Singleton&) = delete;

	public:

		virtual ~Singleton() {}

		static inline Type* GetInstance()
		{
			static Type instance;
			return &instance;
		}
	};
}

#define IMPLEMENT_SINGLETON(Type) Type(){} friend class Core::Singleton<Type>;

#endif