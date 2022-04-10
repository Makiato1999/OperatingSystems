`fsck-exfat` volumes
====================

Here are some volumes that you can use to test your `fsck`.

`exfat-empty-consistent.img`
----------------------------

This volume has no files on it and is consistent: the bitmap and `PercentInUse`
agree.

Expected output:

```
MBR appears to be consistent.
File system appears to be consistent.
```

`exfat-empty-inconsistent-bitmap.img`
-------------------------------------

This volume has no files on it and is inconsistent: the bitmap has marked way
more blocks as in use than `PercentInUse` reports.

Expected output:

```
MBR appears to be consistent.
Inconsistent file system: PercentInUse is 0%, allocation bitmap is 1952/2422 => 80%.
```

`exfat-empty-inconsistent-sig.img`
----------------------------------

This volume has no files on it and is inconsistent: the `BootSignature` is
invalid.

Expected output:

```
Inconsistent file system: BootSignature should be 0xAA55, value is 0x2255.
```

`exfat-empty-inconsistent-volumelength.img`
-------------------------------------------

This volume has no files on it and is inconsistent: the `VolumeLength` field is
too small.

Expected output:

```
Inconsistent file system: VolumeLength should be >2048, value is 16.
```

`exfat-with-files-consistent.img`
---------------------------------

This volume has files on it and is consistent.

Expected output:

```
MBR appears to be consistent.
File system appears to be consistent.
```

`exfat-with-files-inconsistent-bitmap.img`
-----------------------------------

This volume has files on it and is inconsistent: the bitmap and `PercentInUse`
disagree.

Expected output:

```
MBR appears to be consistent.
Inconsistent file system: PercentInUse is 7%, allocation bitmap is 730/2422 => 30%.
```
