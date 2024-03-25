# What files are created when a LevelDB is created?

- LOG
- LOCK
- CURRENT
- 000003.log
- MANIFEST-000002

# What files are touched when key-value data is written to a LevelDB?

After a few writes:
- LOG was touched.
- LOG.old was created.
- 000003.log became 000008.log.
- MANIFEST-000002 became MANIFEST-000006.
- 000007.ldb was created.  This is the sorted table file.  These are created from the log file when it reached a certain size.
- CURRENT was updated to contain the text "MANIFEST-000006".

After a few more writes:
- A new 000010.ldb was created.  Contents identical to 000007.ldb though.
- 000008.log became 000011.log, which contains all the transactions.

# What files are touched when key-value is data is delete from a LevelDB?
After deleting every other key:
- A new 000013.ldb was created, but it doesn't seem to contain the data.
- 000011.log became 000014.log.  Either that or 000011.log was deleted and 000014.log was created.
- 000014.log seems to contain the deleted keys, which makes sense because [the log files are supposed to be logs of recent updates](https://github.com/google/leveldb/blob/main/doc/impl.md#log-files).