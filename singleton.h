/*
 * singleton.h
 *
 *  Created on: May 28, 2015
 *      Author: yanghui
 */

#ifndef SINGLETON_H_
#define SINGLETON_H_


#define DECLARE_SINGLETON(ClassName) \
	private: \
		static ClassName *singleton_; \
	public: \
		static ClassName *GetInstance(); \
		static void ReleaseInstance();

#define IMPLEMENT_SINGLETON(ClassName) \
	ClassName *ClassName::singleton_ = NULL; \
	ClassName *ClassName::GetInstance() { \
		if (singleton_ == NULL) { \
			singleton_ = new ClassName(); \
		} \
		return singleton_; \
	} \
	void ClassName::ReleaseInstance() { \
		if (singleton_ != NULL) { \
			delete singleton_; \
			singleton_ = NULL; \
		} \
	}

#endif /* SINGLETON_H_ */
