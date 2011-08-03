/*  LibMary - C++ library for high-performance network servers
    Copyright (C) 2011 Dmitry Shatrov

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef __LIBMARY__INFORMER__H__
#define __LIBMARY__INFORMER__H__


#include <libmary/types.h>
#include <libmary/object.h>
#include <libmary/virt_ref.h>
#include <libmary/code_ref.h>
#include <libmary/intrusive_list.h>


namespace M {

// This class should not be used from outside.
class GenericInformer : public DependentCodeReferenced
{
public:
    enum InformFlags {
	// TODO Support oneshot subscriptions.
	InformOneshot = 1
    };

protected:
    union CallbackPtr
    {
	void *obj;
	VoidFunction func;

	CallbackPtr (void * const obj) : obj (obj) {}
	CallbackPtr (VoidFunction const func) : func (func) {}
    };

    typedef void (*ProxyInformCallback) (CallbackPtr   cb_ptr,
					 void         *cb_data,
					 VoidFunction  inform_cb,
					 void         *inform_data);

    class Subscription : public IntrusiveListElement<>
    {
    public:
	bool valid;

	GenericInformer *informer;
	bool oneshot;
	Object::DeletionSubscriptionKey del_sbn;

	CallbackPtr cb_ptr;
	void *cb_data;
	WeakCodeRef weak_code_ref;

	VirtRef ref_data;

	Subscription (CallbackPtr      const cb_ptr,
		      void           * const cb_data,
		      VirtReferenced * const ref_data,
		      Object         * const coderef_container)
	    : cb_ptr (cb_ptr),
	      cb_data (cb_data),
	      weak_code_ref (coderef_container),
	      ref_data (ref_data)
	{
	}
    };

public:
    class SubscriptionKey
    {
	friend class GenericInformer;
    private:
	Subscription *sbn;
	SubscriptionKey (Subscription * const sbn) : sbn (sbn) {}
    public:
	operator bool () const { return sbn; }
	SubscriptionKey () : sbn (NULL) {}
    };

protected:
    StateMutex * const mutex;

    mt_mutex (mutex) IntrusiveList<Subscription> sbn_list;
    mt_mutex (mutex) Count traversing;

    void releaseSubscription (Subscription *sbn);

    static void subscriberDeletionCallback (void *_sbn);

    void informAll (ProxyInformCallback  mt_nonnull proxy_inform_cb,
		    VoidFunction         inform_cb,
		    void                *inform_cb_data);

    // May unlock and lock 'mutex' in the process.
    void informAll_unlocked (ProxyInformCallback  mt_nonnull proxy_inform_cb,
			     VoidFunction         inform_cb,
			     void                *inform_cb_data);

    SubscriptionKey subscribeVoid (CallbackPtr     cb_ptr,
				   void           *cb_data,
				   VirtReferenced *ref_data,
				   Object         *coderef_container);

    SubscriptionKey subscribeVoid_unlocked (CallbackPtr     cb_ptr,
					    void           *cb_data,
					    VirtReferenced *ref_data,
					    Object         *coderef_container);

public:
    void unsubscribe (SubscriptionKey sbn_key);

    void unsubscribe_unlocked (SubscriptionKey sbn_key);

    GenericInformer (Object     * const coderef_container,
		     StateMutex * const mutex)
	: DependentCodeReferenced (coderef_container),
	  mutex (mutex),
	  traversing (0)
    {
    }

    ~GenericInformer ();
};

template <class T>
class Informer_ : public GenericInformer
{
public:
    typedef void (*InformCallback) (T    *ev_struct,
				    void *cb_data,
				    void *inform_data);

protected:
    static void proxyInformCallback (CallbackPtr      const cb_ptr,
				     void           * const cb_data,
				     VoidFunction     const inform_cb,
				     void           * const inform_data)
    {
	((InformCallback) inform_cb) ((T*) cb_ptr.obj, cb_data, inform_data);
    }

public:
    void informAll (InformCallback   const inform_cb,
		    void           * const inform_cb_data)
    {
	GenericInformer::informAll (proxyInformCallback, (VoidFunction) inform_cb, inform_cb_data);
    }

    SubscriptionKey subscribe (T              * const ev_struct,
			       void           * const cb_data,
			       VirtReferenced * const ref_data,
			       Object         * const coderef_container)
    {
	return subscribeVoid ((void*) ev_struct, cb_data, ref_data, coderef_container);
    }

    SubscriptionKey subscribe_unlocked (T              * const ev_struct,
					void           * const cb_data,
					VirtReferenced * const ref_data,
					Object         * const coderef_container)
    {
	return subscribeVoid_unlocked ((void*) ev_struct, cb_data, ref_data, coderef_container);
    }

    Informer_ (Object     * const coderef_container,
	       StateMutex * const mutex)
	: GenericInformer (coderef_container, mutex)
    {
    }
};

template <class T>
class Informer : public GenericInformer
{
public:
    typedef void (*InformCallback) (T     cb,
				    void *cb_data,
				    void *inform_data);

protected:
    static void proxyInformCallback (CallbackPtr      const cb_ptr,
				     void           * const cb_data,
				     VoidFunction     const inform_cb,
				     void           * const inform_data)
    {
	((InformCallback) inform_cb) ((T) cb_ptr.func, cb_data, inform_data);
    }

public:
    void informAll (InformCallback   const inform_cb,
		    void           * const inform_cb_data)
    {
	GenericInformer::informAll (proxyInformCallback, (VoidFunction) inform_cb, inform_cb_data);
    }

    // May unlock and lock 'mutex' in the process.
    void informAll_unlocked (InformCallback    const inform_cb,
			     void            * const inform_cb_data)
    {
	GenericInformer::informAll_unlocked (proxyInformCallback, (VoidFunction) inform_cb, inform_cb_data);
    }

    SubscriptionKey subscribe (T                const cb,
			       void           * const cb_data,
			       VirtReferenced * const ref_data,
			       Object         * const coderef_container)
    {
	return subscribeVoid ((VoidFunction) cb, cb_data, ref_data, coderef_container);
    }

    SubscriptionKey subscribe_unlocked (T                const cb,
					void           * const cb_data,
					VirtReferenced * const ref_data,
					Object         * const coderef_container)
    {
	return subscribeVoid_unlocked ((VoidFunction) cb, cb_data, ref_data, coderef_container);
    }

    Informer (Object     * const coderef_container,
	      StateMutex * const mutex)
	: GenericInformer (coderef_container, mutex)
    {
    }
};

}


#endif /* __LIBMARY__INFORMER__H__ */

