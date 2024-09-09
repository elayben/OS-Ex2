#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include "buffered_open.h"

buffered_file_t *buffered_open(const char *pathname, int flags, ...)
{
    buffered_file_t *bf = (buffered_file_t *)malloc(sizeof(buffered_file_t));
    if (!bf)
    {
        errno = ENOMEM;
        return NULL;
    }
     memset(bf, 0, sizeof(buffered_file_t));

    va_list args;
    va_start(args, flags);
    mode_t mode = va_arg(args, mode_t);
    
    va_end(args);

    bf->flags = flags;
    bf->preappend = (flags & O_PREAPPEND) ? 1 : 0;
    bf->flags &= ~O_PREAPPEND;

    bf->fd = open(pathname, bf->flags, mode);

    if (bf->fd == -1)
    {
        // free(bf);
        return NULL;
    }

    bf->read_buffer = (char *)malloc(BUFFER_SIZE);
    bf->write_buffer = (char *)malloc(BUFFER_SIZE);
    if (!bf->read_buffer || !bf->write_buffer)
    {
        close(bf->fd);
        /*
        free(bf->read_buffer);
        free(bf->write_buffer);
        free(bf);
        */
        errno = ENOMEM;
        return NULL;
    }

    bf->read_buffer_size = BUFFER_SIZE;
    bf->write_buffer_size = BUFFER_SIZE;
    bf->read_buffer_pos = bf->read_buffer_size;
    bf->write_buffer_pos = 0;

    return bf;
}

ssize_t buffered_write(buffered_file_t *bf, const void *buf, size_t count)
{
    ssize_t tmp = 0;
    while (count > tmp)
    {
        size_t remaining = bf->write_buffer_size - bf->write_buffer_pos;
        if (count - tmp < remaining)
        {
            remaining = count - tmp;
        }
        memcpy(bf->write_buffer + bf->write_buffer_pos, buf + tmp, remaining);
        bf->write_buffer_pos += remaining;
        tmp += remaining;

        if (bf->write_buffer_pos == bf->write_buffer_size)
        {
            if (buffered_flush(bf) == -1)
            {
                return -1;
            }
        }
    }

    return count;
}

ssize_t buffered_read(buffered_file_t *bf, void *buf, size_t count)
{
    size_t copied = 0;
    if (buffered_flush(bf) == -1)
    {
        return -1;
    }

    if (bf->read_buffer_pos != bf->read_buffer_size)
    {
        size_t available = bf->read_buffer_size - bf->read_buffer_pos;
        size_t to_read = available > count ? count : available;
        memcpy(buf, bf->read_buffer + bf->read_buffer_pos, to_read);
        count -= to_read;
        copied += to_read;
    }
    while (count > 0)
    {
        size_t available = bf->read_buffer_size;
        size_t to_read = available > count ? count : available;
        ssize_t bytes_read = read(bf->fd, bf->read_buffer + bf->read_buffer_pos, to_read);
        if (bytes_read == -1)
        {
            return -1;
        }
       memcpy(buf + copied, bf->read_buffer + bf->read_buffer_pos, bytes_read);
        bf->read_buffer_pos = bf->read_buffer_size;
        copied += bytes_read;
    
        if (bytes_read != 0)
        {
            count -= bytes_read;
        }   
        else
        {
            count = 0;
        }
    
    }
    
     return copied;

}

int buffered_flush(buffered_file_t *bf)
{

    if (bf->write_buffer_pos > 0)
    {
        if (bf->preappend)
        {
            // Use a predefined temporary file path
            const char *temp_filename = "tempfile.txt";
            int temp_fd = open(temp_filename, O_RDWR | O_CREAT | O_TRUNC, 0644);
            if (temp_fd == -1)
            {
                return -1;
            }

            off_t current = lseek(bf->fd, 0, SEEK_CUR);
            // Transfer the current file content to the temporary file using a buffer
            char *temp_buffer = malloc(BUFFER_SIZE);
            ssize_t bytes_read;
            while ((bytes_read = read(bf->fd, temp_buffer, BUFFER_SIZE)) > 0)
            {
                if (write(temp_fd, temp_buffer, bytes_read) == -1)
                {
                    close(temp_fd);
                    unlink(temp_filename);
                    return -1;
                }
            }

            // Handle errors in reading
            if (bytes_read == -1)
            {
                close(temp_fd);
                unlink(temp_filename);
                return -1;
            }

            // Write the new data to the original file
            lseek(bf->fd, current, SEEK_SET);
            if (write(bf->fd, bf->write_buffer, bf->write_buffer_pos) == -1)
            {
                close(temp_fd);
                unlink(temp_filename);
                return -1;
            }

            // Append the data from the temporary file back to the original file
            lseek(temp_fd, 0, SEEK_SET);
            while ((bytes_read = read(temp_fd, temp_buffer, BUFFER_SIZE)) > 0)
            {
                if (write(bf->fd, temp_buffer, bytes_read) == -1)
                {
                    close(temp_fd);
                    unlink(temp_filename);
                    return -1;
                }
            }

            // Handle errors in reading from the temporary file
            if (bytes_read == -1)
            {
                close(temp_fd);
                unlink(temp_filename);
                return -1;
            }

            lseek(bf->fd, bf->write_buffer_pos + current, SEEK_SET);
            // Clean up the temporary file
            close(temp_fd);
            unlink(temp_filename);

            remove(temp_filename);
        }
        else
        {
            lseek(bf->fd, 0, SEEK_END);
            write(bf->fd, bf->write_buffer, bf->write_buffer_pos);
        }

        bf->write_buffer_pos = 0;
    }

    return 0;
}

int buffered_close(buffered_file_t *bf)
{
    if (buffered_flush(bf) == -1)
    {
        return -1;
    }
    int ret = close(bf->fd);
    /*
    free(bf->read_buffer);
    free(bf->write_buffer);
    free(bf);
    */
    return ret;
}
