**pwnbadge** Make the HITB 2018 AMS badge glow red.

Uses [librpitx](https://github.com/F5OEO/librpitx) to transform your pi into
an SDR that triggers the secret red glow functionality of the [HITB 2018 AMS
badge](https://github.com/klks/hitb2018ams_badge) made by WhiteA10n3, xwings
and klks84.

Connect a wire to pin 7 of the GPIO header as antenna.

# Build

	git clone --recurse-submodules https://github.com/iksteen/pwnbadge
	cd pwnbadge
	make

# Run

	sudo ./pwnbadge
