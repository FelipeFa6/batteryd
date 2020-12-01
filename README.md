# batteryd

Battery state watcher daemon for OpenBSD. Runs a script or an application when
the battery state switches to `high`, `low`, or `critical`.

## Usage
```
batteryd [-d]
```
By default, `batteryd` runs as a daemon. If passed the `-d` flag, `batteryd`
enters debug mode, staying in the foreground, and logging output is printed to
`stderr`.

When run as root, it uses `/etc/batteryd` as its configuration directory. When
run as a regular user, it uses `$HOME/.config/batteryd` as its configuration
directory.

The daemon polls `apm(4)` every second to discover changes in the battery charge
level state. Three states are handled: `critical`, `low`, and `high`. When a
change is discovered, an executable bearing the name of the state located in the
configuration directory is executed.

## Files

When run as root:
```
/etc/batteryd/critical
/etc/batteryd/low
/etc/batteryd/high
```
When run as a user:
```
$HOME/.config/batteryd/critical
$HOME/.config/batteryd/low
$HOME/.config/batteryd/high
```
