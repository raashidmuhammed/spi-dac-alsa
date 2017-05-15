#include <linux/module.h>
#include <linux/of.h>
#include <linux/device.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>

#include <linux/spi/spi.h>

static char *id = "za-audio-p1";		/* ID for this card */

#ifdef CONFIG_OF
static const struct of_device_id spidac_dt_ids[] = {
	{ .compatible = "za-audio-p1" },
	{},
};
MODULE_DEVICE_TABLE(of, spidac_dt_ids);
#endif

struct snd_spi_dac {
	struct snd_card			*card;
	struct snd_pcm			*pcm;
	struct snd_pcm_substream	*substream;
	struct spi_device               *spi;
};


static int snd_spi_dac_pcm_open(struct snd_pcm_substream *substream)
{
	return 0;
}

static int snd_spi_dac_pcm_close(struct snd_pcm_substream *substream)
{
	return 0;
}

static struct snd_pcm_ops spi_dac_playback_ops = {
	.open		= snd_spi_dac_pcm_open,
	.close		= snd_spi_dac_pcm_close,
};



static int snd_spi_probe(struct spi_device *spi)
{
	struct snd_card *card;
	struct snd_pcm *pcm;

	int status = 0;


	int retval;

	retval = snd_card_new(&spi->dev, -1, id, THIS_MODULE,
			      sizeof(struct snd_spi_dac), &card);
	if (retval < 0) {
		printk("Error creating new sound card.\n");
		return -1;
	}

	retval = snd_pcm_new(card, card->shortname, 0, 1, 0, &pcm);
	if (retval < 0)	{
		printk("Adding new PCM device failed.\n");
		return retval;
	}

//	pcm->private_data =
	pcm->info_flags = SNDRV_PCM_INFO_BLOCK_TRANSFER;
	strcpy(pcm->name, "spi-dac-pcm");
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, &spi_dac_playback_ops);

	strcpy(card->driver, "spi_dac");
	printk("Register Sound card.\n");
	retval = snd_card_register(card);
	if (retval) {
		printk("Sound card registration failed.\n");
		snd_card_free(card);
		return -1;
	}

	dev_set_drvdata(&spi->dev, card);

	return status;
}

static int snd_spi_remove(struct spi_device *spi)
{
	struct snd_card *card = dev_get_drvdata(&spi->dev);

	snd_card_free(card);

	return 0;
}

static struct spi_driver spi_dac_driver = {
	.driver		= {
		.name	= "vybrid_spi",
		.of_match_table = of_match_ptr(spidac_dt_ids),
	},
	.probe		= snd_spi_probe,
	.remove		= snd_spi_remove,
};

module_spi_driver(spi_dac_driver);

MODULE_AUTHOR("Raashid <raashidmuhammed@zilogic.com>");
MODULE_DESCRIPTION("ALSA Sound driver for SPI DAC");
MODULE_LICENSE("GPL");
