int gas;
int memory[];
int localmem[];
int test(int addr0, int addr1, int addr2, int call_dsize, int call_v, int calldata0, int calldata36, int calldata4, int sha1, int sha2, int sha3) {
	gas = 0;
	int r1, r10, r11, r12, r13, r14, r15, r18, r19, r28, r3, r30, r33, r35, r36, r37, r5, r50, r52, r53, r54, r6, r7, r9;
	r1 = r10 = r11 = r12 = r13 = r14 = r15 = r18 = r19 = r28 = r3 = r30 = r33 = r35 = r36 = r37 = r5 = r50 = r52 = r53 = r54 = r6 = r7 = r9 = 0;
	label1:
		gas = gas + (30);
		// UseMemory operations currently ignored
		localmem[64] = 96;
		r7 = call_dsize < 4 ? 1 : 0;
		if (r7 != 0) {
			goto label2;
		} else {
			goto label3;
		}
	label2:
		gas = gas + (7);
		return 0;
	label3:
		gas = gas + (45);
		r5 = calldata0;
		r10 = r5 / addr0;
		r12 = (addr1 & r10);
		r15 = 447770341 == r12 ? 1 : 0;
		r1 = r12;
		if (r15 != 0) {
			goto label4;
		} else {
			goto label9;
		}
	label4:
		gas = gas + (19);
		r5 = call_v == 0 ? 1 : 0;
		if (r5 != 0) {
			goto label5;
		} else {
			goto label8;
		}
	label5:
		gas = gas + (161);
		r14 = calldata4;
		r30 = calldata36;
		r50 = memory[0];
		r52 = r14 < r50 ? 1 : 0;
		r53 = r52 == 0 ? 1 : 0;
		r54 = r53 == 0 ? 1 : 0;
		r5 = r30;
		r7 = r14;
		if (r54 != 0) {
			goto label6;
		} else {
			goto label7;
		}
	label6:
		gas = gas + (79);
		localmem[0] = 0;
		r15 = sha1;
		r19 = r7 + r15;
		gas = gas + (r5 == 0 ? 5000 : 20000);
		// Refund gas ignored
		memory[r19] = r5;
		return 0;
	label7:
		return 0;
	label8:
		gas = gas + (6);
		return 0;
	label9:
		gas = gas + (22);
		r6 = 1691300792 == r1 ? 1 : 0;
		if (r6 != 0) {
			goto label10;
		} else {
			goto label18;
		}
	label10:
		gas = gas + (19);
		r5 = call_v == 0 ? 1 : 0;
		if (r5 != 0) {
			goto label11;
		} else {
			goto label17;
		}
	label11:
		gas = gas + (27);
		r3 = 0;
		goto label12;
	label12:
		gas = gas + (84);
		r9 = memory[0];
		r14 = r3 < r9 ? 1 : 0;
		r15 = r14 == 0 ? 1 : 0;
		if (r15 != 0) {
			goto label13;
		} else {
			goto label14;
		}
	label13:
		gas = gas + (12);
		return 0;
	label14:
		gas = gas + (84);
		r11 = memory[0];
		r13 = r3 < r11 ? 1 : 0;
		r14 = r13 == 0 ? 1 : 0;
		r15 = r14 == 0 ? 1 : 0;
		if (r15 != 0) {
			goto label15;
		} else {
			goto label16;
		}
	label15:
		gas = gas + (149);
		localmem[0] = 0;
		r14 = sha2;
		r18 = r3 + r14;
		r19 = memory[r18];
		gas = gas + (r19 == 0 ? 5000 : 20000);
		// Refund gas ignored
		memory[1] = r19;
		r28 = 1 + r3;
		r3 = r28;
		goto label12;
	label16:
		return 0;
	label17:
		gas = gas + (6);
		return 0;
	label18:
		gas = gas + (22);
		r6 = addr2 == r1 ? 1 : 0;
		if (r6 != 0) {
			goto label19;
		} else {
			goto label24;
		}
	label19:
		gas = gas + (19);
		r5 = call_v == 0 ? 1 : 0;
		if (r5 != 0) {
			goto label20;
		} else {
			goto label23;
		}
	label20:
		gas = gas + (137);
		r13 = calldata4;
		r33 = memory[0];
		r35 = r13 < r33 ? 1 : 0;
		r36 = r35 == 0 ? 1 : 0;
		r37 = r36 == 0 ? 1 : 0;
		r6 = r13;
		if (r37 != 0) {
			goto label21;
		} else {
			goto label22;
		}
	label21:
		gas = gas + (179);
		localmem[0] = 0;
		r14 = sha3;
		r18 = r6 + r14;
		r19 = memory[r18];
		// UseMemory operations currently ignored
		localmem[96] = r19;
		return 0;
	label22:
		return 0;
	label23:
		gas = gas + (6);
		return 0;
	label24:
		gas = gas + (7);
		return 0;

}

void main(int addr0, int addr1, int addr2, int call_dsize, int call_v, int calldata0, int calldata36, int calldata4, int sha1, int sha2, int sha3) {
	test(addr0, addr1, addr2, call_dsize, call_v, calldata0, calldata36, calldata4, sha1, sha2, sha3);
	__VERIFIER_print_hull(gas);
	return;
}
