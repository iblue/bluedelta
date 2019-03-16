# Binary diff and patching

This tool works well with files such as disk images, where data usually stays
on the same position unless changed.

## Usage

To create a patch file from two files:

```
bluedelta ./file1.img ./file2.img ./patch.img
```

To restore `file2.img` from a patch, use `-r`:

```
bluedelta -r ./file1.img ./file2.img ./patch.img
```
