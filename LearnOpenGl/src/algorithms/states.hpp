#ifndef STATES_HPP
#define STATES_HPP

/*
	namespace to tie together all states functions
*/

namespace States {
	/*
		for example
		states = 10001101
	*/


	// check state
	template<typename T>
	bool isIndexActive(T* states, int target) {

		/*
			target = 6
			1 = 00000001
			1 << target = 01000000

			10001101 &
			01000000
			00000000
		*/

		return (*states & (1 << target));
	}

	// activate state
	template<typename T>
	void activateIndex(T* states, int target) {
		/*
			target 4
			1 = 00000001
			1 << target = 00010000

			10001101 
			00010000 |
			---------
			10011101
		*/
		*states |= 1 << target;
	}

	// uniquely activate state (no other can be activated)
	template<typename T>
	void uniquelyActivateIndex(T* states, int target) {
		/*
			target 4
			1 = 00000001
			1 << target = 00010000

			10001101
			00010000 |
			---------
			10011101
			00010000 &
			---------
			00010000
		*/

		activateIndex<T>(states, target); // activate state first
		*states &= 1 << target; // deactivate other
	}

	// deactivate states
	template<typename T>
	void deactivateIndex(T* states, int target) {
		/*
			target 3
			1 = 00000001
			1 << target = 00001000
			~(1 << target) = 11110111

			10001101
			11110111 &
			---------
			10000101
		*/
		*states &= ~(1 << target);
	}

	// toggle state
	template<typename T>
	void toggleIndex(T* states, int target) {
		/*
			target 4
			1 = 00000001
			1 << target = 00010000

			10001101
			00010000 XOR
			---------
			10011101
		*/
		*states ^= 1 << target;
	}

	// tell if state is active
	template<typename T>
	bool isActive(T* states, T state) {
		return (*states & state);
	}

	// activate state
	template<typename T>
	void activate(T* states, T state) {
		*states |= state;
	}

	// uniquely activate state (no others can be active)
	template<typename T>
	void uniquelyActivate(T* states, T state) {
		*states &= state;
	}

	// deactivate state
	template<typename T>
	void deactivate(T* states, T state) {
		*states &= ~state;
	}

	// toggle state
	template<typename T>
	void toggle(T* states, T state) {
		*states ^= state;
	}
}

#endif // !STATES_HPP
