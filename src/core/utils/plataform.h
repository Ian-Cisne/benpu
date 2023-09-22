#ifdef _WIN32
	#ifdef _WIN64
		#define BENPU_WINDOWS_PLATAFORM
	#else
		#error "32-bits is not suppported."
	#endif
#elif defined(__APPLE__)
		#error "32-bits is not suppported."
#elif defined(__ANDROID__)
		#error "32-bits is yet not suppported."
#elif defined(__linux__)
	#define BENPU_LINUX_PLATAFORM
#else
	#error "Couldn't recognize plataform."
#endif