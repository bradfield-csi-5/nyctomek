# Send an emoji to the browser from `nc`

```
# Start nc in server mode listening on port 12345.
$ nc -l 1235

# In a browser navigate to localhost:12345.

# Back in nc:
HTTP/1.1 200 OK
Content-length 4
Content-type: text/plain;charset=utf8

😍

# Observe the browser rendering an emoji.
```

