/*! @file       ReadWriteBuffer.h
    @version    2.0
    @brief      Byte buffer management

    Management of a byte buffer. Backing store may be either an aliased buffer
    owned outside this class, or an internal growable buffer owned by this class.
 */

#ifndef INCLUDED_ReadWriteBuffer
#define INCLUDED_ReadWriteBuffer

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// ----------------------------------------------------------------------------
// ReadWriteBuffer

/*! @brief Basic buffer with read/write cursors. 

    The buffer may be backed by either an internally allocated and managed
    growable buffer, or a statically sized external buffer may be used.

    Example:
    <pre>
        ReadWriteBuffer buf;

        char * p = buf.GetWriteBuffer(sizeof(data1));
        memcpy(p, data1, sizeof(data1));
        buf.CommitWriteBytes(sizeof(data1));

        p = buf.GetWriteBuffer(sizeof(data2));
        memcpy(p, data2, sizeof(data2));
        buf.CommitWriteBytes(sizeof(data2));

        while (buf.GetReadSize() > 0) {
            fputc(*buf.GetReadBuffer(), fp);
            buf.CommitReadBytes(1);
        }
    </pre>
 */
class ReadWriteBuffer 
{
public:
    /*! @brief constructor */
    ReadWriteBuffer();

    /*! @brief Create an internal buffer copy of the supplied buffer.
        
        Only the committed data will be copied. The available bytes may be different 
        to the source buffer. 
     */
    ReadWriteBuffer(const ReadWriteBuffer &);

    /*! @brief Copy the supplied buffer to an internal buffer.

        Only the committed data will be copied. The available bytes may be 
        different to the source buffer. 
     */
    ReadWriteBuffer & operator=(const ReadWriteBuffer &);

    /*! @brief destructor */
    ~ReadWriteBuffer();

    /*! @brief Deallocate and clear out any existing buffer */
    void Deallocate();

    /*! @brief Clear the existing buffer to zero lengths for read and write buffers */
    void SetEmpty();

    /*! @brief Set the buffer as an alias to an external buffer. 

        @param a_pBuf       Buffer to use as external store.
        @param a_nBufSiz    Maximum size of the buffer for writing.
        @param a_nBufLen    Current length of readable data in buffer.
        @param a_nGrowBy    When 0, running out of space in the buffer throws
                            an exception. When > 0, the buffer is automatically
                            converted to an internal buffer with this growby value.

        @throw std::invalid_argument
    */
    void SetExternalBuffer(void * a_pBuf, size_t a_nBufSiz, size_t a_nBufLen = 0, size_t a_nGrowBy = 0); // throw std::invalid_argument 

    /*! @brief Replace the current buffer with an internal growable buffer. 
        
        Internal buffers will be grown by adding a_nMinBytes rounded 
        up to an even multiples of a_nGrowBy. 

        @param a_nInitialSize   Initial size to create the internal buffer at
        @param a_nGrowBy        Number of bytes to grow the buffer by whenever
                                it runs out of space.
     */
    void SetInternalBuffer(size_t a_nInitialSize, size_t a_nGrowBy = 1024); // throw std::bad_alloc

    /*! @brief Get a write pointer into the buffer guaranteeing that the specified number
        of bytes are available. 
        
        Call CommitWriteBytes() after accessing the buffer to specify the number of 
        bytes actually written. 
     
        @param a_nMinBytes  Minimum writable size required for the returned pointer.
                            If 0 then the currently available buffer will be returned
                            without resizing it. The available bytes for writing can
                            be determined via GetWriteSize().

        @throw std::overflow_error  when an external buffer is used, conversion to an
                                    internal buffer on overflow is disabled, and 
                                    a_nMinBytes is not available in the buffer.
        @throw std::bad_alloc       when using an internal buffer and there was an out
                                    of memory error on resizing.

        @return Writable pointer to a buffer with at least the requested number of 
                bytes available for writing. See the function GetWriteSize() to find
                the actual number of bytes available.
     */
    char * GetWriteBuffer(size_t a_nMinBytes = 0); // throw std::overflow_error, std::bad_alloc

    /*! @brief Commit the specified number of bytes to the buffer. 
    
        This function should be called after the buffer returned by GetWriteBuffer() is 
        written to. After calling this function, the specified bytes will then be available 
        for reading from the GetReadBuffer() and GetReadSize() functions.
        
        @param a_nBytes     Number of bytes written

        @throw std::invalid_argument    if more bytes were committed then is available
     */
    void CommitWriteBytes(size_t a_nBytes); // throw std::invalid_argument

    /*! @brief Get the available buffer size for writing.

        @return number of bytes available in the write buffer returned 
                from GetWriteBuffer()
     */
    size_t GetWriteSize() const;

    /*! @brief Write the supplied data to the write buffer. 
    
        This function is a simple wrapper around calls to GetWriteBuffer, 
        memcpy, CommitWriteBytes. 

        @param a_pBuf       Data to copy to the buffer
        @param a_nBufLen    Length of the data in bytes to copy

        @throw std::overflow_error  when an external buffer is used and a_nBufLen is 
                                    not available.
        @throw std::bad_alloc       when using an internal buffer and there was an out
                                    of memory error on resizing.
     */
    void WriteBytes(const void * a_pBuf, size_t a_nBufLen); // throw std::overflow_error, std::bad_alloc

    /*! @brief Get the read buffer. 
    
        See GetReadSize() for the number of bytes available to be read 
        from this pointer.

        @return Pointer to the data to be read.
     */
    const char * GetReadBuffer() const;

    /*! @brief Update the read bytes available.
    
        Indicate that the specified number of bytes have been read and are not 
        necessary anymore. These bytes are now available for reclaiming by 
        Compact(). 

        @param a_nBytes     Number of bytes read

        @throw std::invalid_argument    if more bytes were committed then is available
     */
    void CommitReadBytes(size_t a_nBytes); // throw std::invalid_argument

    /*! @brief Get the available buffer size for reading.
    
        @return number of bytes available for reading 
    */
    size_t GetReadSize() const;

    /*! @brief Read the data from the buffer. 
    
        This function is a simple wrapper around calls to GetReadSize, GetReadBuffer, 
        memcpy, CommitReadBytes. 

        @param a_pBuf       Location for the buffer data to be copied to
        @param a_nBufLen    Length of the data in bytes to copy

        @throw std::invalid_argument    not enough bytes in the buffer
     */
    void ReadBytes(void * a_pBuf, size_t a_nBufLen); // throw std::invalid_argument

    /*! @brief Compact the buffer.
    
        All remaining unread bytes are moved to the beginning of the buffer leaving 
        the largest size available for writing. 
     */
    void Compact();

    /*! @brief Do a memcmp on the read buffers */
    bool operator==(const ReadWriteBuffer &) const;

private:
    size_t mBufSiz; //!< max buf size
    size_t mBufLen; //!< committed writes
    size_t mBufIdx; //!< committed reads
    size_t mGrowBy; //!< growby rate
    bool   mIsExt;  //!< is this an external buffer?
    char * mBuf;    //!< buffer
};

#endif // INCLUDED_ReadWriteBuffer
