
#ifdef CONFIG_COOPERATIVE
static int vnet_host_get_mac(struct pci_dev *pdev,unsigned char *addr) {
	pci_read_config_byte(pdev, PCI_CO_MAC1, &addr[0]);
	pci_read_config_byte(pdev, PCI_CO_MAC2, &addr[1]);
	pci_read_config_byte(pdev, PCI_CO_MAC3, &addr[2]);
	pci_read_config_byte(pdev, PCI_CO_MAC4, &addr[3]);
	pci_read_config_byte(pdev, PCI_CO_MAC5, &addr[4]);
	pci_read_config_byte(pdev, PCI_CO_MAC6, &addr[5]);
	return 0;
}
static int vnet_host_tx(int host_unit, void *data, int len) {
	co_send_message(CO_MODULE_LINUX,
			CO_MODULE_VNET0 + host_unit,
			CO_PRIORITY_DISCARDABLE,
			CO_MESSAGE_TYPE_OTHER,
			len,
			data);
}
static irqreturn_t vnet_interrupt(int irq, void *dev_id)
{
	co_message_node_t *node_message;

	spin_lock(&vnet_irq_lock);
	while (co_get_message(&node_message, CO_DEVICE_NETWORK)) {
		struct net_device *dev;
		co_linux_message_t *message;

		message = (co_linux_message_t *)&node_message->msg.data;
		if (message->unit < 0  ||  message->unit >= CO_MODULE_MAX_VNET) {
			printk("vnet interrupt: buggy network reception unit %d\n", message->unit);
			spin_unlock(&vnet_irq_lock);
			return IRQ_HANDLED;
		}

		dev = vnet_dev[message->unit];
		if (!dev) {
			co_free_message(node_message);
			continue;
		}

		if (!netif_running(dev)) {
			co_free_message(node_message);
			continue;
		}

		if (node_message->msg.type == CO_MESSAGE_TYPE_STRING) {
			int connected= *(int*)(message+1);
			if (connected)
				netif_carrier_on(dev);
			else
				netif_carrier_off(dev);
			co_free_message(node_message);
			continue;
		}

		if (message->len > 0x10000) {
			printk("vnet rx: buggy network reception\n");
			priv->stats.rx_dropped++;
			return;
		} else
			vnet_rx(dev, message->data, message->len);

		co_free_message(node_message);
	}
	spin_unlock(&vnet_irq_lock);

	return IRQ_HANDLED;
}
