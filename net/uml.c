
static int vnet_host_get_mac(struct pci_dev *pdev,unsigned char *addr) {
	struct vnet_priv *priv = dev_get_drvdata(&pdev->dev);

//	return os_net_get_mac(priv->host_unit, addr);
	return -1;
}

static int vnet_host_tx(int host_unit, void *data, int len) {
	/* Send straight to host */
//	return os_net_tx(host_unit, data, len);
	return -1;
}

#if 0
void vmnet_host_rx(struct net_dev *dev) {
	struct vnet_private *priv = dev->priv;
	int i;

	spin_lock(&vnet_irq_lock);
	printk("***vmnet_host_rx: host_unit: %d\n", host_unit);
	for(i=0; i < VNET_NUM_UNITS; i++) {
		if (vnet_dev[i]->host_unit == host_unit) {
			printk("***vmnet_host_rx: found.\n");
			vnet_rx(vnet_dev[i]->dev, data, len);
			break;
		}
	}
	spin_unlock(&vnet_irq_lock);
}
#endif
irqreturn_t vnet_interrupt(int irq, void *dev_id)
{
#if 0
	struct net_device *dev = dev_id;
	struct vnet_private *priv = dev->priv;
	int err;

	if(!netif_running(dev)) return(IRQ_NONE);

//	spin_lock(&lp->lock);
	while((err = vnet_rx(dev)) > 0) ;
	if(err < 0) {
		printk(KERN_ERR
		       "Device '%s' read returned %d, shutting it down\n",
		       dev->name, err);
		/* dev_close can't be called in interrupt context, and takes
		 * again lp->lock.
		 * And dev_close() can be safely called multiple times on the
		 * same device, since it tests for (dev->flags & IFF_UP). So
		 * there's no harm in delaying the device shutdown.
		 * Furthermore, the workqueue will not re-enqueue an already
		 * enqueued work item. */
		schedule_work(&lp->work);
		goto out;
	}
	reactivate_fd(priv->fd, dev->irq);

out:
//	spin_unlock(&lp->lock);
#endif
	return IRQ_HANDLED;
}
static int vnet_host_open(struct net_device *dev)
{
#if 0
	struct vnet_priv *priv = (struct vnet_priv *)dev->priv;
	int rc;

	if(priv->host_unit >= 0){
		rc = -ENXIO;
		goto out;
	}

//	priv->host_unit = os_net_open();
	priv->host_unit = -1;
	if(priv->host_unit < 0){
		err = priv->host_unit;
		goto out;
	}

	rc = um_request_irq(dev->irq, priv->host_unit, IRQ_READ, vnet_interrupt,
			     IRQF_DISABLED | IRQF_SHARED, dev->name, dev);
	if(rc != 0){
		printk(KERN_ERR "vnet_open: failed to get irq(%d)\n", err);
		rc = -ENETUNREACH;
		goto out_close;
	}

//	lp->tl.data = (unsigned long) &lp->user;
	netif_start_queue(dev);

	/* clear buffer - it can happen that the host side of the interface
	 * is full when we get here.  In this case, new data is never queued,
	 * SIGIOs never arrive, and the net never works.
	 */
	while((rc = vnet_rx(dev)) > 0) ;

#if 0
	spin_lock(&opened_lock);
	list_add(&lp->list, &opened);
	spin_unlock(&opened_lock);
#endif

	return 0;

out_close:
	os_net_close(priv->host_unit);
	priv->host_unit = -1;
out:
	return rc;
#endif
	return 0;
}

static int vnet_host_close(struct net_device *dev)
{
#if 0
	struct vnet_private *priv = dev->priv;

	netif_stop_queue(dev);

	free_irq(dev->irq, dev);
	os_net_close(priv->host_unit);
	priv->host_unit = -1;

	spin_lock(&opened_lock);
	list_del(&lp->list);
	spin_unlock(&opened_lock);
#endif
	return 0;
}
