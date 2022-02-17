
static int vnet_open(struct net_device *dev)
{
	struct vnet_priv *priv = netdev_priv(dev);
	int rc;

	if (priv->flags & VNET_FLAG_ENABLED) return 0;

	if (vnet_host_open(dev)) return rc;

	priv->flags |= VNET_FLAG_ENABLED;

	netif_start_queue(dev);

	return 0;
}

static int vnet_stop(struct net_device *dev)
{
	struct vnet_priv *priv = netdev_priv(dev);

	if ((priv->flags & VNET_FLAG_ENABLED) == 0) return 0;

	vnet_host_close(dev);
//	netif_stop_queue(dev);

	priv->flags &= ~VNET_FLAG_ENABLED;

	return 0;
}

static int vnet_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	int len;
	char *data;
	struct vnet_priv *priv = netdev_priv(dev);

	len = skb->len < ETH_ZLEN ? ETH_ZLEN : skb->len;
	data = skb->data;

	dev->trans_start = jiffies; /* save the timestamp */

	vnet_host_tx(priv->host_unit, data, len);

	priv->stats.tx_bytes+=skb->len;
	priv->stats.tx_packets++;

	dev_kfree_skb(skb);

	return 0;
}

static void vnet_rx(struct net_device *dev, void *data, int len) {
	struct sk_buff *skb;
	struct vnet_priv *priv = netdev_priv(dev);

	/*
	 * The packet has been retrieved from the transmission
	 * medium. Build an skb around it, so upper layers can handle it
	 */
	skb = dev_alloc_skb(len+2);
	if (!skb) {
		printk("vnet rx: low on mem - packet dropped\n");
		priv->stats.rx_dropped++;
		return;
	}

	memcpy(skb_put(skb, len), data, len);

	/* Write metadata, and then pass to the receive level */
	skb->dev = dev;
	skb->protocol = eth_type_trans(skb, dev);
	skb->ip_summed = CHECKSUM_NONE; /* make the kernel calculate and verify
                                           the checksum */

	priv->stats.rx_bytes += len;
	priv->stats.rx_packets++;

	netif_rx(skb);
	return;
}

static struct net_device_stats* vnet_get_stats(struct net_device *dev)
{
	struct vnet_priv *priv = netdev_priv(dev);

	return &priv->stats;
}

static int vnet_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	/* We support 100-baseT Full duplex TP */
	cmd->port = PORT_TP;
	cmd->duplex = DUPLEX_FULL;
	cmd->supported = SUPPORTED_TP | SUPPORTED_100baseT_Full;
	cmd->speed = SPEED_100;
	return 0;
}

static int vnet_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	/* We support TP Full duplex 100 */
	if (cmd->port != PORT_TP || cmd->duplex != DUPLEX_FULL || cmd->speed != SPEED_100)
		return -EOPNOTSUPP;
	return 0;
}

static void vnet_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
	struct vnet_priv *priv = netdev_priv(dev);

	strcpy(info->driver, DRV_NAME);
	strcpy(info->version, DRV_VERSION);
#ifdef CONFIG_PCI
	strcpy(info->bus_info, pci_name(priv->pdev));
#else
	strcpy(info->bus_info, "platform");
#endif
}

static u32 vnet_get_link(struct net_device *dev)
{
	/* Always connected */
	return 1;
}

static u32 vnet_get_msglevel(struct net_device *dev)
{
	struct vnet_priv *priv = netdev_priv(dev);

        return ((priv->flags & VNET_FLAG_DEBUG) != 0);
}

static void vnet_set_msglevel(struct net_device *dev, u32 level)
{
	struct vnet_priv *priv = netdev_priv(dev);

	if (level)
		priv->flags |= VNET_FLAG_DEBUG;
	else
		priv->flags &= ~VNET_FLAG_DEBUG;
}

static int vnet_mdio_read(struct net_device *dev, int id, int reg)
{
	struct vnet_priv *priv = netdev_priv(dev);
	int val;

	if (priv->flags & VNET_FLAG_DEBUG)
		printk(KERN_INFO "vnet%d: mdio_read: id: %d, reg: %d\n", priv->unit, id, reg);
	switch(reg) {
	case MII_BMCR:			/* Basic mode control register */
		val = BMCR_FULLDPLX | BMCR_SPEED100;
		break;
	case MII_BMSR:			/* Basic mode status register  */
		val = BMSR_LSTATUS | BMSR_100FULL;
		break;
	default:
		val = 0;
		break;
	}
	return val;
}

static void vnet_mdio_write(struct net_device *dev, int id, int reg, int val)
{
	struct vnet_priv *priv = netdev_priv(dev);

	if (priv->flags & VNET_FLAG_DEBUG)
		printk(KERN_INFO "vnet%d: mdio_write: id: %d, reg: %d, val: %d\n", priv->unit, id, reg, val);
	return;
}

static int vnet_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct vnet_priv *priv = netdev_priv(dev);
	int rc;

	spin_lock(&priv->ioctl_lock);
	rc = generic_mii_ioctl(&priv->mii_if, if_mii(rq), cmd, NULL);
	spin_unlock(&priv->ioctl_lock);

	return rc;
}

static struct ethtool_ops vnet_ethtool_ops = {
	.get_settings           = vnet_get_settings,
	.set_settings           = vnet_set_settings,
	.get_drvinfo            = vnet_get_drvinfo,
	.get_link               = vnet_get_link,
	.get_msglevel           = vnet_get_msglevel,
	.set_msglevel           = vnet_set_msglevel,
#if 0
	.nway_reset             = vnet_nway_reset,
	.get_ringparam          = vnet_get_ringparam,
	.set_ringparam          = vnet_set_ringparam,
	.get_tx_csum            = ethtool_op_get_tx_csum,
	.get_sg                 = ethtool_op_get_sg,
	.get_tso                = ethtool_op_get_tso,
	.get_strings            = vnet_get_strings,
	.self_test_count        = vnet_self_test_count,
	.self_test              = vnet_ethtool_test,
	.phys_id                = vnet_phys_id,
	.get_regs_len           = vnet_get_regs_len,
	.get_regs               = vnet_get_regs,
	.get_perm_addr          = ethtool_op_get_perm_addr,
#endif
};

static struct net_dev *init_adapter(struct device *bus_dev) {
	int host_unit;
	unsigned char addr[6];
	struct net_device *dev;
	struct vnet_priv *priv;
	int rc;

	dprintk(KERN_INFO "VNET: probing!\n");

//	host_unit = os_net_open();
	host_unit = -1;
	printk("host_unit: %d\n", host_unit);
	if (host_unit < 0) {
		panic("os_net_open");
		return 0;
	}

//	unit = units++;

	dev = alloc_etherdev(sizeof(*priv));
	if (dev == NULL) {
		printk(KERN_ERR "VNET: could not allocate memory for device.\n");
		return 0;
	}
//	SET_MODULE_OWNER(dev);
	SET_NETDEV_DEV(dev, bus_dev);

	spin_lock_init(&priv->ioctl_lock);
	spin_lock_init(&vnet_irq_lock);

#if 0
	dev->open = vnet_open;
	dev->stop = vnet_stop;
	dev->hard_start_xmit = vnet_hard_start_xmit;
	dev->ethtool_ops = &vnet_ethtool_ops;
	dev->get_stats = vnet_get_stats;
	dev->do_ioctl = vnet_ioctl;
//	dev->irq = VNET_IRQ;
#endif

	priv = netdev_priv(dev);
	priv->dev = dev;
	priv->unit = dev->ifindex;
	priv->host_unit = host_unit;

	/* Get MAC addr from host */
#if 0
	vnet_host_get_mac(pdev,addr);
	dprintk("VNET: eth%d: addr: %02x:%02x:%02x:%02x:%02x:%02x\n", dev->ifindex,
		addr[0],addr[1],addr[2],addr[3],addr[4],addr[5]);
	memcpy(&dev->dev_addr,addr,6);
#endif

	priv->mii_if.full_duplex = 1;
	priv->mii_if.phy_id_mask = 0x1f;
	priv->mii_if.reg_num_mask = 0x1f;
	priv->mii_if.dev = dev;
	priv->mii_if.mdio_read = vnet_mdio_read;
	priv->mii_if.mdio_write = vnet_mdio_write;
	priv->mii_if.phy_id = 1;

	mii_link_ok(&priv->mii_if);

	rc = register_netdev(dev);
	if (rc) {
		printk(KERN_ERR "VNET: could not register device; rc: %d\n", rc);
		goto error_out_dev;
	}

//	vnet_dev[dev->ifindex] = dev;
	vnet_dev[dev->ifindex] = priv;

#if 0
	printk(KERN_INFO "vnet%d: irq %d, HWAddr %02x:%02x:%02x:%02x:%02x:%02x\n",
		unit, NETWORK_IRQ, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
#endif

	return priv;

error_out_dev:
	free_netdev(dev);
	return 0;
}
