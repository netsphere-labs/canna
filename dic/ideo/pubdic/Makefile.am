
iroha.t: $(PUBDICS) ProgramTargetName(pod)
	cat $(PUBDICS) | ./pod - -p -i -2 > iroha.spl
	$(MERGE) < iroha.spl > iroha.t
	$(RM) iroha.spl

