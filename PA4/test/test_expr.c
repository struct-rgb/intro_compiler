
int g[5];

int get(int array[], int index) {
	return array[index];
}

int pointerize(int ref[]) {
	return get(ref, 3);
}

void main(void) {

	int e[5];

	e[3] = 69;
	g[3] = 69;
	
	output(g[3]);
	output(pointerize(g));
	output(e[3]);
	output(pointerize(e));
}
