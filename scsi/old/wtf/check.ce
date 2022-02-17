
static int check_condition(struct vscsi_device *dp, int key, int asc, int asq) {
	dp->key = key;
	dp->asc = asc;
	dp->asq = asq;
	return CHECK_CONDITION;
}
