//*****************************************************************************
/*!
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *****************************************************************************
 *
 *  \file   refcount.h
 *
 *  \brief  Smart pointers
 *
 *  \version
 *      - S Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SMARTPTR_H_
#define _SMARTPTR_H_

/**
 * Superclass of all reference countable objects.
 */
class RefCountable
{
public:
    //! Constructor
    RefCountable() : refCount(0) { }

    //! virtual destructor
    virtual ~RefCountable() { }

    //! Increase reference count
    virtual void IncRef(unsigned delta = 1)
    {
        refCount += delta;
    }

    //! Decrease reference count
    // Returns true if reference count reaches 0
    virtual bool    DecRef(unsigned delta = 1)
    {
        // TODO: Should we assert on refCount < delta?
        if (refCount > delta)
        {
            refCount -= delta;
            return false;
        }

        refCount = 0;
        return true;
    }

    //! Return the reference count
    virtual unsigned Count() const { return refCount; }

private:
    //! ref count of this object
    unsigned    refCount;
};

//*****************************************************************************
/*!
 *  \class CRefCount
 *
 *  \brief  Base-Class for all objects that need to be ref-counted so they
 *  can be automatically freed when out of scope (and fashion)!
 *
 *  TODO: Specify Locking policy
 *
 *****************************************************************************/
template <typename T>
class CRefCount
{
public:
    // Constructor
    CRefCount(T *data) : refData(data), refCount(1) {  }

    // Destructor
    inline ~CRefCount() { delete refData; }

    //! Increase reference count
    inline void    IncRef(unsigned delta = 1) { refCount += delta; }

    //! Decrease reference count
    // Returns true if reference count reaches 0
    inline bool    DecRef(unsigned delta = 1)
    {
        // TODO: Should we assert on refCount < delta?
        if (refCount > delta)
        {
            refCount -= delta;
            return false;
        }

        refCount = 0;
        return true;
    }

    //! Return the reference count
    inline unsigned Count() const { return refCount; }

    //! Get the data held within
    inline T* Data() const { return refData; }

private:
    //! Constructor
    //! TODO: Enable pooling of objects to avoid 
    //  unnecessary creations and destructions.
    CRefCount();

private:
    T *         refData;
    unsigned    refCount;
};

//*****************************************************************************
/*!
 *  \class CSmartPtr
 *
 *  \brief  A holder for reference counted objects
 *
 *****************************************************************************/
template <typename T>
class CSmartPtr
{
public:
    // Default Constructor - creates a NULL pointer
    CSmartPtr() { Reset(); }

    //! Constructor
    CSmartPtr(const T *obj) : rcObject(new CRefCount<T>(const_cast<T*>(obj))), isConst(true) { }

    //! Constructor
    CSmartPtr(T *obj, bool is_const = false) :
        rcObject(new CRefCount<T>(obj)),
        isConst(is_const && obj != NULL) { }

    //! Copy constructor for constant objects
    CSmartPtr(const CSmartPtr &val) { Reset(); CopyAnother(val); }

    //! Destructor
    ~CSmartPtr()
    {
        if ( ! isConst && rcObject && rcObject->DecRef())
        {
            delete rcObject;
        }
    }

    //! Reset the smart poitner
    inline void Reset() { rcObject = NULL; isConst = true; }

    //! Assignment operator
    CSmartPtr & operator=(const CSmartPtr &another)
    {
        if (this != &another)
            CopyAnother(another);
        return *this;
    }

    //! Bool not operator
    inline bool operator!() const { return !rcObject || !rcObject->Data(); }

    //! Dereferencing operator
    inline T *operator->() const { return rcObject->Data(); }

    //! Return the value data
    inline T *Data() const { return rcObject ? rcObject->Data() : NULL; }

private:
    // Copy another object
    void CopyAnother(const CSmartPtr &another)
    {
        if (!isConst && rcObject && rcObject->DecRef())
            delete rcObject;
        rcObject = another.rcObject;
        isConst  = another.isConst;

        if (!isConst && rcObject)
            rcObject->IncRef();
    }

private:
    // Object reference count
    CRefCount<T> *  rcObject;

    //! Is the object constant
    bool            isConst;
};

#endif

