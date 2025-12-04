install: 
	pip install datetime --break-system-packages
	sudo cp bftoc.py /bin
	sudo chmod +x /bin/bftoc.py
	sudo ${CC} main.c -o /bin/4letbf -lm
