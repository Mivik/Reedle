
## Reedle

Inject into other apps, based on [Riru](https://github.com/RikkaApps/Riru).

## Usage

Put `.so` files in `/data/data/[Target Package Name]/files/reedle/[Architecture]/`, Reedle will load it and call `void reedle_main(const char *app_data_dir)` if it exists at the start of the app.

You could update `.so` files anytime, and you just need to restart the app to apply the changes. This greatly speeds up debugging.
