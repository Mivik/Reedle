
## Reedle

Inject into other apps easily, based on [Riru](https://github.com/RikkaApps/Riru).

## Usage

Execute

```shell
./gradlew assembleRelease
```

and module file will then be built in `/out` directory. Enable it on Magisk and then reboot your phone.

Put library files in `/data/data/[Target Package Name]/files/reedle/[Architecture]/`, Reedle will load it and call `void reedle_main(JNIEnv *env, const char *app_data_dir)` if it exists at the start of the app.

You could update library files anytime, and you just need to restart the app to apply the changes. This greatly speeds up debugging.

Note that you should ensure the app process has access to the library file.
