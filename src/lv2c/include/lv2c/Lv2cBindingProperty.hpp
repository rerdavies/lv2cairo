// Copyright (c) 2023 Robin E. R. Davies
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once
#include <functional>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <concepts>
#include <exception>
#include <stdexcept>
#include <concepts>
#include <memory>

#include "Lv2cElement.hpp"

namespace lv2c
{
    class Lv2cElement;

    template <typename T>
        requires std::equality_comparable<T>
    class Observable;

    namespace implementation
    {
        // private
        extern uint64_t nextHandle();
        // variables for testing only! Not threadsafe.
        extern uint64_t handleCount;
        extern uint64_t bindingRecordCount;
        extern uint64_t observerLinkCount;

        /// @brief Private use.
        class ObserverLink
        {
        public:
            ObserverLink();
            virtual ~ObserverLink();
            void ObserverDeleted();
            void ObservableDeleted();

        protected:
            virtual void OnObserverDeleted() {}

        private:
            bool observerValid = true;
            bool observableValid = true;
        };
        // private implementation of observer_handle_t
        class ObserverHandle
        {
        public:
            // movable, not copyable.
            ObserverHandle(const ObserverHandle &) = delete;
            ObserverHandle &operator=(const ObserverHandle &other) = delete;

            ObserverHandle();

            ObserverHandle(ObserverLink *link);
            ObserverHandle(ObserverHandle &&other);

            ObserverHandle &operator=(ObserverHandle &&other);
            ~ObserverHandle();
            void Release();

        private:
            ObserverLink *link;
        };

    }

    class BindingTransform
    {
    public:
        using self = BindingTransform;
        using ptr = std::shared_ptr<self>;

        virtual ~BindingTransform() noexcept {}
        virtual double Forward(double value) = 0;
        virtual double Reverse(double value) = 0;
    };
    class IdentityBindingTransform : public BindingTransform
    {
    public:
        using self = IdentityBindingTransform;
        using super = BindingTransform;
        static ptr Create() { return std::make_shared<self>(); }
        virtual double Forward(double value) { return value; }
        virtual double Reverse(double value) { return value; }
    };

    class MapRangeBindingTransform : public BindingTransform
    {
    public:
        virtual ~MapRangeBindingTransform() noexcept {}
        using self = MapRangeBindingTransform;
        using super = BindingTransform;
        using ptr = std::shared_ptr<self>;

        static ptr Create(double sourceMinVal, double sourceMaxVal, double targetMinVal, double targetMaxVal);
        MapRangeBindingTransform(double sourceMinVal, double sourceMaxVal, double targetMinVal, double targetMaxVal);

        virtual double Forward(double value) override;
        virtual double Reverse(double value) override;

    private:
        double sourceMinVal;
        double sourceMaxVal;
        double targetMinVal;
        double targetMaxVal;

    };

    /// @brief Returns a trait that determines whether arguments should be passed by value.
    /// @tparam T A type.
    /// By default, intrinsic types define arg_t to be T (pass by value); all other types
    /// have a Lv2cArgumentTraits<T>::arg_t of type const T& (pass by const reference).
    ///
    /// Other types that should be passed by value can be modifed using a specialization
    /// of
    template <typename T>
    struct Lv2cArgumentTraits
    {
        using arg_t = const T &;
        using value_t = T;
        using move_t = T &&;
        static constexpr bool move_disabled = false;
        static constexpr bool move_enabled = true;
    };

    template <typename T>
        requires std::integral<T> || std::floating_point<T> || std::is_enum_v<T>
    struct Lv2cArgumentTraits<T>
    {
        using arg_t = T;
        using value_t = T;
        using move_t = struct _disabled
        {
        };
        static constexpr bool move_disabled = true;
        static constexpr bool move_enabled = false;
    };

    /// @brief Flags controlling notification to the owning element of a Lv2cBindingProperty.
    /// @see  operator+(Lv2cBindingFlags v1, Lv2cBindingFlags v2),
    ///        operator-(Lv2cBindingFlags v1, Lv2cBindingFlags v2),
    ///        operator&&(Lv2cBindingFlags v1, Lv2cBindingFlags v2),
    ///        operator&&(Lv2cBindingFlags v1, Lv2cBindingFlags v2),

    enum class Lv2cBindingFlags
    {
        // No notifications.
        Empty = 0,
        // Call Lv2cObject::Invalidate when the value changes.
        InvalidateOnChanged = 1,
        // Call Lv2cObject::InvalidateLayout when the value changes.
        InvalidateLayoutOnChanged = 2,
    };

    // Set all flags in v2.
    inline Lv2cBindingFlags operator+(Lv2cBindingFlags v1, Lv2cBindingFlags v2)
    {
        return (Lv2cBindingFlags)(((int)v1) | ((int)v2));
    }
    // Unset all flags in v2.
    inline Lv2cBindingFlags operator-(Lv2cBindingFlags v1, Lv2cBindingFlags v2)
    {
        return (Lv2cBindingFlags)(((int)v1) & ~((int)v2));
    }
    // returns true if all of the flags in v2 are set in v1.
    inline bool operator&&(Lv2cBindingFlags v1, Lv2cBindingFlags v2)
    {
        return (((int)v1) & ((int)v2)) == (int)v2;
    }
    // returns true if any of the flags in v2 are set in v1.
    inline bool operator||(Lv2cBindingFlags v1, Lv2cBindingFlags v2)
    {
        return (((int)v1) & ((int)v2)) != 0;
    }

    /// @brief Handle for an observation. See Observable<T>::addObserver
    /// Note that the observation is valid only for the lifetime of the
    /// observer_handle_t object.. Deleting the observer_handle_t removes the observation!
    ///
    /// An observer_handle_t is moveable, but not copyable.
    ///
    /// @see Observable::addObserver().

    using observer_handle_t = implementation::ObserverHandle;

    /// @brief A callback for Observable::addObserver.
    /// A callback function matcing the signature `void (const T&value)` for
    //  compount types, or void (T) for intrinsic types (integers, floats, enums).
    template <typename T>
    using ObserverCallback = std::function<void(const T &value)>;

    template <typename T>
        requires std::equality_comparable<T>
    class Observable
    {
    public:
        using arg_t = typename Lv2cArgumentTraits<T>::arg_t;
        using move_t = typename Lv2cArgumentTraits<T>::move_t;
        using type_t = T;
        using observer_callback_t = ObserverCallback<T>;
        using handle_t = observer_handle_t;

        // not movable, not copyable.
        Observable(const Observable<T> &) = delete;
        Observable(Observable<T> &&) = delete;

        /// @brief Constructor.
        /// @param initialValue The initial value of the observable.
        Observable();
        Observable(arg_t initialValue);
        Observable(move_t initialValue)
            requires Lv2cArgumentTraits<T>::move_enabled;
        /// @brief Destructor.

        virtual ~Observable();

        /// @brief Observe changes to the value of the Observable<T>. See important note regarding the handling of the return value.
        /// @param observerCallback The function<> to call when the value changes.
        /// @return A handle used to manage the lifetime of the observation.
        /// IMPORTANT: The returned observer_handle_t manages the lifetime of the observation.
        ///
        /// Prefer the version of the method which takes an `ObserverCallback<T>&&` argument, since capture variables
        /// are copied instead of moved in the `const ObserverCallback<T>&` variant.
        ///
        /// The observation remains valid only for the lifetime of the returned observer_handle_t. If
        /// the handle is destroyed, the link to the Observable is terminated. You MUST assign the returned handle
        /// to an instance of observer_handle_t that remains alive for the duration of the desired notification.
        ///
        /// observer_handle_t is movable, but not copyable. Moving an observer_handle_t to another observer_handle_t transfers
        /// ownership of the observation to the target observer_handle_t.
        ///
        /// It is specifically not an error if the lifetime of the observer_handle_t exceeds the lifetime of
        /// the observable. Although obviously, if the  Observable is destroyed, the callback won't get any
        /// further calls.
        ///
        /// The subcription will be terminated by any of:
        ///
        /// - Calling observer_handle_t.Release on the returned handle.
        /// - Destroying the observer_handle_t.
        /// - Assigning a default observer_handle t();
        /// - Calling Observable<T>::removeObserver (which simply calls observer_handle_t::Release()).
        ///
        [[nodiscard]] observer_handle_t addObserver(const ObserverCallback<T> &observerCallback);
        [[nodiscard]] observer_handle_t addObserver(ObserverCallback<T> &&observerCallback);
        /// @brief Remove the observation.
        /// @param handle
        /// A convenient method which calls handle.Release(). Prefer handle.Release().
        void removeObserver(observer_handle_t &handle);

        /// @brief Get the current value of the observable.
        /// @return The current value.
        arg_t get() const;
        /// @brief Set the current value of the observable, notifying andy observers.
        /// @param value The value to set the observable to.
        /// For compelx types, prefer the `T&&value` variant.
        void set(arg_t value);

        void set(move_t value)
            requires Lv2cArgumentTraits<T>::move_enabled;

        /// @brief The current number of observers.
        /// @return
        /// Primarily for test use.
        size_t observerCount() const { return observers.size(); }

    protected:
        virtual void on_changed(arg_t value)
        {
        }

    private:
        friend class Link;
        void RemoveHandle(uint64_t handle);

        class Link : public implementation::ObserverLink
        {
        public:
            Link(
                Observable<T> *observable,
                uint64_t handle,
                const ObserverCallback<T> &observerCallback);
            Link(
                Observable *observable,
                uint64_t handle,
                ObserverCallback<T> &&observerCallback);
            virtual void OnObserverDeleted() override;

            Observable<T> *observable;
            uint64_t handle;
            ObserverCallback<T> observerCallback;
        };

        std::unordered_map<uint64_t, Link *> observers;
        T value = T();
    };

    template <typename T>
    class Lv2cBindingProperty : public Observable<T>
    {
    public:
        // T, for intrinsics, const T& for everything else.
        using arg_t = typename Lv2cArgumentTraits<T>::arg_t;
        // disabled for intrinsics, T&& for everything else.`
        using move_t = typename Lv2cArgumentTraits<T>::move_t;
        using super = Observable<T>;
        using type_t = T;
        using observable_t = Observable<T>;
        using observer_callback_t = ObserverCallback<T>;

        Lv2cBindingProperty() {}
        // not movable, not copyable
        Lv2cBindingProperty(Lv2cBindingProperty<T> &&) = delete;
        Lv2cBindingProperty(const Lv2cBindingProperty<T> &) = delete;

        Lv2cBindingProperty(arg_t value) : super(value) {}
        Lv2cBindingProperty(move_t value)
            requires Lv2cArgumentTraits<T>::move_enabled
            : super(value)
        {
        }

        ~Lv2cBindingProperty()
        {
            while (myBindings.size() != 0)
            {
                myBindings[0]->remove();
            }
            while (theirBindings.size() != 0)
            {
                theirBindings[0]->remove();
            }
        }

        void Bind(Lv2cBindingProperty &target) { Bind(*this, target); }
        void Bind(Lv2cBindingProperty &target, BindingTransform::ptr bindingTransform) requires std::is_same_v<T,double>
         { Bind(*this, target, bindingTransform); }

        using OnChangedMemberPointer = void (Lv2cObject::*)(arg_t value);

        /// @brief Set the ownling element for for this binding site.
        /// @param element Pointer to class that owns the current property.
        /// @param bindingFlags Flags controlling invalidation and layout invalidation.
        /// @param methodPtr Method pointer to the method that will be called.
        /// @param methodPtr Method pointer to the method that will be called.
        /// `element` should be the Lv2cObject that declares the Lv2cBindingProperty. These
        /// methods provide convenient hook for common element notifications. Setting
        /// the controlling element of a Lv2cBindingProperty is completely optional.

        void SetElement(Lv2cElement *element, Lv2cBindingFlags bindingFlags);
        void SetElement(Lv2cObject *element, OnChangedMemberPointer methodPtr);
        void SetElement(Lv2cElement *element, Lv2cBindingFlags bindingFlags, OnChangedMemberPointer methodPtr);

        // provide implicit conversion of member pointers.
        template <typename U>
            requires std::derived_from<U, Lv2cObject>
        void SetElement(U *this_, void (U::*methodPtr)(arg_t))
        {
            SetElement((Lv2cObject *)this_,
                       (OnChangedMemberPointer)methodPtr);
        }
        // provide implicit conversion of member pointers.
        template <typename U>
            requires std::derived_from<U, Lv2cElement>
        void SetElement(U *this_, Lv2cBindingFlags bindingFlags, void (U::*methodPtr)(arg_t))
        {
            SetElement((Lv2cElement *)this_, bindingFlags,
                       (OnChangedMemberPointer)methodPtr);
        }

    protected:
        virtual void on_changed(arg_t value) override
        {
            if (parentElement)
            {
                if (elementOnChangedPtr)
                {
                    (((Lv2cElement*)parentElement)->*elementOnChangedPtr)(value);
                }
                if (invalidateLayoutMemberPtr)
                {
                    (((Lv2cElement*)parentElement)->*invalidateLayoutMemberPtr)();
                }
                if (invalidateMemberPtr)
                {
                    (((Lv2cElement*)parentElement)->*invalidateMemberPtr)();
                }
            }
        }

    private:
        Lv2cObject *parentElement = nullptr;
        OnChangedMemberPointer elementOnChangedPtr = 0;

        using OnInvalidateMemberPointer = void (Lv2cElement::*)();
        OnInvalidateMemberPointer invalidateMemberPtr = 0;
        OnInvalidateMemberPointer invalidateLayoutMemberPtr = 0;

    public:
        struct BindingRecord
        {
            BindingRecord() { ++implementation::bindingRecordCount; }
            ~BindingRecord() { --implementation::bindingRecordCount; }

            BindingRecord(
                Lv2cBindingProperty *pSource,
                Lv2cBindingProperty *pTarget,
                observer_handle_t &&sourceHandle,
                observer_handle_t &&targetHandle)
                : pSource(pSource), pTarget(pTarget), sourceHandle(std::move(sourceHandle)), targetHandle(std::move(targetHandle))
            {
                ++implementation::bindingRecordCount;
            }
            Lv2cBindingProperty *pSource;
            Lv2cBindingProperty *pTarget;
            observer_handle_t sourceHandle;
            observer_handle_t targetHandle;
            void remove()
            {
                remove(pSource->myBindings, this);
                remove(pTarget->theirBindings, this);
                delete this; // releases the handles.
            }

            static void remove(std::vector<BindingRecord *> &vec, BindingRecord *value)
            {
                for (auto i = vec.begin(); i != vec.end(); ++i)
                {
                    if ((*i) == value)
                    {
                        vec.erase(i);
                        return;
                    }
                }
            }
        };

        using BindingHandle = struct _BindingHandle
        {
            /* opaque*/
        } *;

        static BindingHandle Bind(Lv2cBindingProperty &source, Lv2cBindingProperty &target, BindingTransform::ptr bindingTransform)
        requires std::is_same_v<T,double> // binding transform ONLY for double variables.
        {
            Lv2cBindingProperty *pSource = &source;
            Lv2cBindingProperty *pTarget = &target;
            if (!bindingTransform)
            {
                bindingTransform = IdentityBindingTransform::Create();
            }
            pTarget->set(bindingTransform->Forward(pSource->get()));
            observer_handle_t sourceHandle = source.addObserver(
                [pTarget,bindingTransform](arg_t value)
                { 
                    if (bindingTransform->Reverse(pTarget->get()) == value) // guard against circular updates due to rounding errors.
                    {
                        return;
                    }
                    pTarget->set(bindingTransform->Forward(value));
                });
            observer_handle_t targetHandle = target.addObserver(
                [pSource,bindingTransform](arg_t value)
                {
                    if (bindingTransform->Forward(pSource->get()) == value) // guard against circular updates due to rounding errors.
                    {
                        return;
                    }
                    pSource->set(bindingTransform->Reverse(value));
                });
            BindingRecord *bindingRecord = new BindingRecord{
                pSource, pTarget,
                std::move(sourceHandle), std::move(targetHandle)};
            pSource->myBindings.push_back(bindingRecord);
            pTarget->theirBindings.push_back(bindingRecord);
            return (BindingHandle)(void *)bindingRecord;
        }
        static BindingHandle Bind(Lv2cBindingProperty &source, Lv2cBindingProperty &target)
        {
            Lv2cBindingProperty *pSource = &source;
            Lv2cBindingProperty *pTarget = &target;

            pTarget->set(pSource->get());

            observer_handle_t sourceHandle = source.addObserver(
                [pTarget](arg_t value)
                { 
                    pTarget->set(value);
                });
            observer_handle_t targetHandle = target.addObserver(
                [pSource](arg_t value)
                {
                    pSource->set(value);
                });
            BindingRecord *bindingRecord = new BindingRecord{
                pSource, pTarget,
                std::move(sourceHandle), std::move(targetHandle)};
            pSource->myBindings.push_back(bindingRecord);
            pTarget->theirBindings.push_back(bindingRecord);
            return (BindingHandle)(void *)bindingRecord;
        }

        void Unbind(BindingHandle handle)
        {
            BindingRecord *bindingRecord = (BindingRecord *)(void *)handle;
            bindingRecord->remove();
        }
        size_t bindingCount() const { return myBindings.size() + theirBindings.size(); }

    private:
        std::vector<BindingRecord *> myBindings;
        std::vector<BindingRecord *> theirBindings;
    };

    //////////////////////////// inlines ///////////////////////////////////

    template <typename T>
    void Lv2cBindingProperty<T>::SetElement(
        Lv2cObject *this_,
        Lv2cBindingProperty<T>::OnChangedMemberPointer method)
    {
        this->parentElement = this_;
        this->elementOnChangedPtr = method;
        this->invalidateLayoutMemberPtr = 0;
        this->invalidateMemberPtr = 0;
    }
    template <typename T>
    void Lv2cBindingProperty<T>::SetElement(Lv2cElement *element, Lv2cBindingFlags bindingFlags)
    {
        this->parentElement = element;
        elementOnChangedPtr = 0;
        invalidateLayoutMemberPtr =
            (bindingFlags && Lv2cBindingFlags::InvalidateLayoutOnChanged)
                ? (&Lv2cElement::InvalidateLayout)
                : 0;

        invalidateMemberPtr =
            (bindingFlags && Lv2cBindingFlags::InvalidateOnChanged)
                ? (&Lv2cElement::Invalidate)
                : 0;
    }
    template <typename T>
    void Lv2cBindingProperty<T>::SetElement(Lv2cElement *element, Lv2cBindingFlags bindingFlags, OnChangedMemberPointer methodPtr)
    {
        this->parentElement = element;
        this->elementOnChangedPtr = methodPtr;
        invalidateLayoutMemberPtr =
            (bindingFlags && Lv2cBindingFlags::InvalidateLayoutOnChanged)
                ? (&Lv2cElement::InvalidateLayout)
                : 0;

        invalidateMemberPtr =
            (bindingFlags && Lv2cBindingFlags::InvalidateOnChanged)
                ? (&Lv2cElement::Invalidate)
                : 0;
    }

    template <typename T> requires std::equality_comparable<T>
    observer_handle_t Observable<T>::addObserver(const ObserverCallback<T> &observerCallback)
    {
        uint64_t h_ = implementation::nextHandle();
        Link *link = new Link(this, h_, observerCallback);
        observers[h_] = link;
        return implementation::ObserverHandle(link);
    }
    template <typename T> requires std::equality_comparable<T>
    observer_handle_t Observable<T>::addObserver(ObserverCallback<T> &&observerCallback)
    {
        uint64_t h_ = implementation::nextHandle();
        Link *link = new Link(this, h_, std::move(observerCallback));
        observers[h_] = link;
        return implementation::ObserverHandle(link);
    }
    template <typename T> requires std::equality_comparable<T>
    void Observable<T>::removeObserver(observer_handle_t &handle)
    {
        handle.Release();
    }
    template <typename T> requires std::equality_comparable<T>

    typename Observable<T>::arg_t Observable<T>::get() const
    {
        return value;
    }

    template <typename T> requires std::equality_comparable<T>
    void Observable<T>::set(arg_t value)
    {
        if (value != this->value)
        {
            this->value = value;
            for (auto &observerCallback : observers)
            {
                observerCallback.second->observerCallback(this->value);
            }
            on_changed(this->value);
        }
    }
    template <typename T> requires std::equality_comparable<T>
    void Observable<T>::set(move_t value)
        requires Lv2cArgumentTraits<T>::move_enabled
    {
        if (Lv2cArgumentTraits<T>::move_enabled)
        {
            if (value != this->value)
            {
                this->value = std::move(value);
                for (auto &observerCallback : observers)
                {
                    observerCallback.second->observerCallback(this->value);
                }
                on_changed(this->value);
            }
        }
    }

    template <typename T> requires std::equality_comparable<T>
    Observable<T>::Observable()
    {
    }

    template <typename T> requires std::equality_comparable<T>
    Observable<T>::Observable(arg_t initialValue)
    {
        this->value = initialValue;
    }

    template <typename T> requires std::equality_comparable<T>
    Observable<T>::Observable(move_t initialValue)
        requires Lv2cArgumentTraits<T>::move_enabled
    {
        this->value = std::move(initialValue);
    }
    template <typename T> requires std::equality_comparable<T>
    Observable<T>::~Observable()
    {
        for (auto &f : observers)
        {
            f.second->ObservableDeleted();
        }
    }

    template <typename T> requires std::equality_comparable<T>
    void Observable<T>::RemoveHandle(uint64_t handle)
    {
        auto f = observers.find(handle);
        if (f != observers.end())
        {
            f->second->ObservableDeleted();
            observers.erase(f);
        }
    }
    template <typename T> requires std::equality_comparable<T>
    Observable<T>::Link::Link(Observable<T> *observable, uint64_t handle, const ObserverCallback<T> &observerCallback)
        : observable(observable), handle(handle), observerCallback(observerCallback)
    {
    }
    template <typename T> requires std::equality_comparable<T>
    Observable<T>::Link::Link(Observable *observable, uint64_t handle, ObserverCallback<T> &&observerCallback)
        : observable(observable), handle(handle), observerCallback(std::move(observerCallback))
    {
    }
    template <typename T> requires std::equality_comparable<T>
    void Observable<T>::Link::OnObserverDeleted()
    {
        observerCallback = nullptr;
        observable->RemoveHandle(handle);
    }

// Declare a Lv2cBindingProperty, with gettter and setter that take a value-type argument.
#define BINDING_PROPERTY(NAME, TYPE, DEFAULT_VALUE)           \
    Lv2cBindingProperty<TYPE> NAME##Property{DEFAULT_VALUE}; \
    self &NAME(TYPE value)                                    \
    {                                                         \
        NAME##Property.set(value);                            \
        return *this;                                         \
    }                                                         \
    TYPE NAME() const { return NAME##Property.get(); }

// Declare a Lv2cBindingProperty, with gettter and setter for a const&TYPE value.
#define BINDING_PROPERTY_REF(NAME, TYPE, DEFAULT_VALUE)       \
    Lv2cBindingProperty<TYPE> NAME##Property{DEFAULT_VALUE}; \
    self &NAME(const TYPE &value)                             \
    {                                                         \
        NAME##Property.set(value);                            \
        return *this;                                         \
    }                                                         \
    const TYPE &NAME() const { return NAME##Property.get(); }

    ///////////////////////////////////////
    /// explicit specializations.
    extern template class Observable<double>;
    extern template class Lv2cBindingProperty<double>;
    extern template class Observable<float>;
    extern template class Lv2cBindingProperty<float>;
    extern template class Observable<bool>;
    extern template class Lv2cBindingProperty<bool>;
    extern template class Observable<std::string>;
    extern template class Lv2cBindingProperty<std::string>;

} // namespace
