targets += playwav

PLAYWAV_OBJ := util/playwav.o
ALL_OBJ += $(PLAYWAV_OBJ)

playwav: $(PLAYWAV_OBJ)
	@$(LINK.o) $(LDFLAGS) $(PLAYWAV_OBJ) -o $@
