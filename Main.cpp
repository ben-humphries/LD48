#include "Dawn/Dawn.h"


class DawnApp : public Dawn::Application {
public:
	DawnApp() {
	}

	void onUpdate() {

	}

	void onClose() {

	}
};

int main() {
	DawnApp app = DawnApp();
	app.start();
}